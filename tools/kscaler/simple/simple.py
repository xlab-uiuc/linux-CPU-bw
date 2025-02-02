from bcc import BPF
import numpy as np
import ctypes
import sys

class BoundData(ctypes.Structure):
    _fields_ = [
        ("runtime", ctypes.c_ulonglong),
        ("yieldtime", ctypes.c_ulonglong),
    ]

MAX_HISTORY = 10
class AgnosticData(ctypes.Structure):
    _fields_ = [
        ("len", ctypes.c_int),
        ("last_runtime", ctypes.c_ulonglong * MAX_HISTORY),
        ("last_yieldtime", ctypes.c_ulonglong * MAX_HISTORY),
    ]

bound_l = {
    "runtime": [],
    "yieldtime": []
}

agnostic_l = {
    "runtime": [],
    "yieldtime": []
}

def analyze(runtime, yieldtime):
    runtime_p99 = np.percentile(runtime, 99)
    yieldtime_p99 = np.percentile(yieldtime, 99)

    return [runtime_p99,yieldtime_p99]

b = BPF(src_file="simple.bpf.c")

reco_bound = []

def recommend_bound(cpu, data, size):
    global reco_bound
    bound_event = ctypes.cast(data, ctypes.POINTER(BoundData)).contents
    bound_l["runtime"].append(bound_event.runtime)
    bound_l["yieldtime"].append(bound_event.yieldtime)

    if (len(bound_l["runtime"]) > 4):
        reco_bound = analyze(bound_l["runtime"], bound_l["yieldtime"])
        print(f"Recommendation bound{reco_bound}")
        bound_l["runtime"] = []
        bound_l["yieldtime"] = []

def recommend_agnostic(cpu, data, size):
    agnos_event = ctypes.cast(data, ctypes.POINTER(AgnosticData)).contents
    temp_runtime = []
    temp_yieldtime = []
    for i in range(agnos_event.len):
        temp_runtime.append(agnos_event.last_runtime[i])
        temp_yieldtime.append(agnos_event.last_yieldtime[i])

    # Find local maxima first
    runtime_p99 = np.percentile(temp_runtime, 99)
    yieldtime_p99 = np.percentile(temp_yieldtime, 99)

    agnostic_l["runtime"].append(runtime_p99)
    agnostic_l["yieldtime"].append(yieldtime_p99)

    if (len(agnostic_l["runtime"]) > 4):
        reco_agnostic = analyze(agnostic_l["runtime"], agnostic_l["yieldtime"])
        print(f"Recommendation agnostic{reco_agnostic}")
        print(f"Recommendation bound--{reco_bound}")
        agnostic_l["runtime"] = []
        agnostic_l["yieldtime"] = []

        # Make the final recommendation
        pb_cpu = reco_bound[0] / (reco_bound[0] + reco_bound[1])
        pa_cpu = reco_agnostic[0] / (reco_agnostic[0] + reco_agnostic[1])
        if (pa_cpu < pb_cpu):
            quota =  reco_agnostic[0]
            period = reco_agnostic[0] + reco_agnostic[1]
        else:
            quota =  reco_bound[0]
            period = reco_bound[0] + reco_bound[1]
        print(f"---Final recommendation: quota: {quota} period: {period}---")

# Open the perf buffer and start polling
b["bound_events"].open_perf_buffer(recommend_bound)
b["agnostic_events"].open_perf_buffer(recommend_agnostic)
print("Tracing kscaler_bound_record... Hit Ctrl-C to end.")

# Poll the perf buffer
try:
    while True:
        b.perf_buffer_poll()
except KeyboardInterrupt:
    sys.exit()
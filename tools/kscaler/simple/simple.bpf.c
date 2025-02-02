#include <uapi/linux/ptrace.h>

#define MAX_HISTORY 10

struct trace_event_kscaler_bound_record {
    u64 runtime;
    u64 yieldtime;
};

struct trace_event_kscaler_agnostic_se_record {
    int len;
    u64 __aligned(8) last_runtime[MAX_HISTORY];
    u64 __aligned(8) last_yieldtime[MAX_HISTORY];
};

BPF_PERF_OUTPUT(bound_events);
BPF_PERF_OUTPUT(agnostic_events);

TRACEPOINT_PROBE(kscaler, kscaler_bound_record) {
    struct trace_event_kscaler_bound_record record = {};

    record.runtime = args->runtime;
    record.yieldtime = args->yield;

    bound_events.perf_submit(args, &record, sizeof(record));

    return 0;
}

TRACEPOINT_PROBE(kscaler, kscaler_agnostic_se_record) {
    struct trace_event_kscaler_agnostic_se_record record = {};
    int len = args->len;

    record.len = len;

	// The &args->variable_name offset didn't work so manually calcuated.
	// XXX: find a better way to do this
	bpf_probe_read_kernel(&record.last_runtime, sizeof(record.last_runtime), (void *)args + 16);
	bpf_probe_read_kernel(&record.last_yieldtime, sizeof(record.last_yieldtime), (void *)args + 96);

	agnostic_events.perf_submit(args, &record, sizeof(record));

    return 0;
}

import argparse
import subprocess

class KscalerConfig:
    def __init__(self):
        self.parser = parser = argparse.ArgumentParser(description="Kscaler parser")
        self.cgroup_loc = ""
        self.args = ""

    def kscaler_init(self):
        self.parser.add_argument("--container-name", type=str, required=True, help="container name to observe")
        self.parser.add_argument("--action", type=str, required=False, help="Action to perform. observe vs autotune. Default(observe)", default="observe")

        self.args = self.parser.parse_args()
        return self.args

    def get_cgroup(self):
        try:
            result = subprocess.run(
                ["docker", "inspect", "--format", "{{.Id}}", self.args.container_name],
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            docker_id = result.stdout.strip()[:5]
        except subprocess.CalledProcessError as e:
            print(f"Error running docker inspect: {e.stderr}")
            return None

        try:
            find_command = f"find /sys/fs/cgroup -type d -name '*{docker_id}*'"
            find_result = subprocess.run(
                find_command,
                shell=True,
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            self.cgroup_loc = find_result.stdout.strip()

            return self.cgroup_loc
        except subprocess.CalledProcessError as e:
            print(f"Error running docker inspect: {e.stderr}")
            return None

    def file_write(self, f, value):
        try:
            with open(f, 'w') as file:
                file.write(value)
            print(f"Value '{value}' written to {f}")
        except IOError as e:
            print(f"Error writing to file {f}: {e}")

    def kscaler_activate(self):
        self.file_write(self.cgroup_loc + "/cpu.trace.status", str(1))

    def kscaler_apply(self, quota, period):
        val = f"{quota/1000} {period/1000}"
        self.file_write(self.cgroup_loc + "/cpu.max", val)


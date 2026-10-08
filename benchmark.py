import subprocess
import re

SERVERS = {
    "PocketDB": 6379,
    "Redis": 6380
}

CLIENTS = [1, 10, 50, 100]
REQUESTS = 10000

for clients in CLIENTS:
    print(f"\n--- {clients} concurrent clients ---")

    for name, port in SERVERS.items():
        command = [
            "redis-benchmark",
            "-h", "127.0.0.1",
            "-p", str(port),
            "-t", "set,get",
            "-n", str(REQUESTS),
            "-c", str(clients),
            "-q"
        ]

        result = subprocess.run(
            command,
            capture_output=True,
            text=True
        )

        if result.returncode != 0:
            print(f"{name}: Benchmark failed")
            print(result.stderr)
            continue

        print(f"\n{name}")

        for line in result.stdout.splitlines():
            if "requests per second" in line:
                print(line.strip())

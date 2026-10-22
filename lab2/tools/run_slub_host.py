#!/usr/bin/env python3
"""Build and run the real allocator with ASan/UBSan; leave no repository binary."""

import os
from pathlib import Path
import shlex
import subprocess
import tempfile


def main():
    root = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix="lab2-slub-host-") as directory:
        binary = str(Path(directory) / "slub-host")
        command = [
            *shlex.split(os.environ.get("HOSTCC", "gcc")),
            "-std=gnu99", "-O1", "-g", "-Wall", "-Werror",
            "-Wno-unused-parameter", "-fno-builtin", "-fno-pie", "-no-pie",
            "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
            "-Ilibs", "-Ikern/mm", "-Ikern/debug",
            "tools/test_slub_host.c", "kern/mm/slub.c", "kern/mm/slub_test.c",
            "kern/mm/default_pmm.c", "-o", binary,
        ]
        subprocess.run(command, cwd=root, check=True)
        environment = os.environ.copy()
        environment["ASAN_OPTIONS"] = "detect_leaks=0:halt_on_error=1"
        environment["UBSAN_OPTIONS"] = "halt_on_error=1:print_stacktrace=1"
        positive = subprocess.run(
            [binary], cwd=root, env=environment, text=True,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30,
        )
        print(positive.stdout, end="", flush=True)
        if positive.returncode != 0 or "Host SLUB: PASS" not in positive.stdout:
            raise SystemExit("Host SLUB positive checks failed")

        cases = {
            "foreign": "slab != NULL && slab->magic == SLUB_MAGIC",
            "interior": "offset % slab->cache->object_size == 0",
            "large-interior": "ptr == (char *)slab + DATA_OFFSET",
            "large-tail": "slab != NULL && slab->magic == SLUB_MAGIC",
            "double-live-slab": "slab->occupied[index / 8] & mask",
            "double-empty-slab": "slab != NULL && slab->magic == SLUB_MAGIC",
            "double-large": "slab != NULL && slab->magic == SLUB_MAGIC",
        }
        for case, expected in cases.items():
            result = subprocess.run(
                [binary, case], cwd=root, env=environment, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=10,
            )
            if (result.returncode != 86 or "HOST PANIC" not in result.stdout
                    or expected not in result.stdout
                    or "AddressSanitizer" in result.stdout
                    or "runtime error:" in result.stdout):
                print(result.stdout, end="")
                raise SystemExit(f"Expected panic check failed: {case}")
            print(f"Host SLUB invalid free: {case} PASS", flush=True)
        print("Host SLUB ASan/UBSan: PASS (positive + 7 expected panics)")


if __name__ == "__main__":
    main()

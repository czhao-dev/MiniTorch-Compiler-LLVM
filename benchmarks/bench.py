from __future__ import annotations

import argparse
import csv
from pathlib import Path
from time import perf_counter


def run_placeholder() -> list[dict[str, str]]:
    start = perf_counter()
    elapsed_ms = (perf_counter() - start) * 1000.0
    return [
        {
            "experiment": "frontend_smoke",
            "pytorch_eager_ms": "",
            "minitorch_no_opt_ms": f"{elapsed_ms:.6f}",
            "minitorch_opt_ms": "",
            "speedup_vs_eager": "",
        }
    ]


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", default="benchmarks/results.csv")
    args = parser.parse_args()

    rows = run_placeholder()
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(rows[0].keys()))
        writer.writeheader()
        writer.writerows(rows)

    for row in rows:
        print(row)


if __name__ == "__main__":
    main()


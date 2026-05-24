#!/usr/bin/env python3

from __future__ import annotations

import argparse
import csv
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt

DATASET_TITLES = {
    "random": "Случайный массив",
    "reverse": "Обратно отсортированный массив",
    "nearly_sorted": "Почти отсортированный массив",
}

ALGORITHM_TITLES = {
    "quick_sort": "QuickSort",
    "merge_sort": "MergeSort",
    "string_quick_sort": "String QuickSort",
    "lcp_merge_sort": "LCP MergeSort",
    "msd_radix_sort": "MSD Radix Sort",
    "hybrid_msd_radix_sort": "Hybrid MSD Radix Sort",
}

METRICS = {
    "mean_us": ("Среднее время сортировки, мкс", "time"),
    "char_comparisons": ("Число посимвольных сравнений", "comparisons"),
    "char_accesses": ("Число чтений символов", "accesses"),
}


def read_results(path: Path):
    grouped = defaultdict(lambda: defaultdict(list))
    with path.open(newline="", encoding="utf-8") as source:
        for row in csv.DictReader(source):
            dataset = row["dataset"]
            algorithm = row["algorithm"]
            grouped[dataset][algorithm].append(
                {
                    "size": int(row["size"]),
                    "mean_us": float(row["mean_us"]),
                    "char_comparisons": int(row["char_comparisons"]),
                    "char_accesses": int(row["char_accesses"]),
                }
            )
    for algorithms in grouped.values():
        for rows in algorithms.values():
            rows.sort(key=lambda item: item["size"])
    return grouped


def build_chart(grouped, dataset: str, field: str, ylabel: str, output: Path) -> None:
    plt.figure(figsize=(11, 6))
    for algorithm, rows in grouped[dataset].items():
        plt.plot(
            [row["size"] for row in rows],
            [row[field] for row in rows],
            marker="o",
            markersize=3,
            label=ALGORITHM_TITLES.get(algorithm, algorithm),
        )
    dataset_title = DATASET_TITLES.get(dataset, dataset)
    plt.title(f"{dataset_title}: {ylabel.lower()}")
    plt.xlabel("Количество строк в массиве")
    plt.ylabel(ylabel)
    plt.grid(True, alpha=0.3)
    plt.legend(title="Алгоритм")
    plt.tight_layout()
    plt.savefig(output, dpi=170)
    plt.close()


def main() -> None:
    parser = argparse.ArgumentParser(description="Построение графиков эксперимента сортировки строк")
    parser.add_argument("csv_path", type=Path, help="Путь к файлу results.csv")
    parser.add_argument("--output", type=Path, default=Path("charts"), help="Каталог для PNG-графиков")
    args = parser.parse_args()

    grouped = read_results(args.csv_path)
    args.output.mkdir(parents=True, exist_ok=True)
    for dataset in sorted(grouped):
        for field, (ylabel, prefix) in METRICS.items():
            build_chart(grouped, dataset, field, ylabel, args.output / f"{prefix}_{dataset}.png")
    print(f"Сохранено графиков: {len(grouped) * len(METRICS)}. Каталог: {args.output}")


if __name__ == "__main__":
    main()

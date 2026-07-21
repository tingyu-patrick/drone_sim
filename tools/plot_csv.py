import csv
import sys

import matplotlib

matplotlib.use("Agg")  # 沒有視窗環境（例如在終端機/CI 裡跑）也能存檔
import matplotlib.pyplot as plt


def read_csv(path):
    with open(path, newline="") as f:
        reader = csv.reader(f)
        header = next(reader)
        columns = {name: [] for name in header}
        for row in reader:
            for name, value in zip(header, row):
                columns[name].append(float(value))
    return header, columns


def main():
    csv_path = sys.argv[1] if len(sys.argv) > 1 else "output//particle_simulation.csv"
    out_path = sys.argv[2] if len(sys.argv) > 2 else "output//drone_plot.png"

    header, columns = read_csv(csv_path)
    t = columns[header[0]]

    fig, ax = plt.subplots(figsize=(8, 4.5))
    for name in header[1:]:
        ax.plot(t, columns[name], label=name)

    ax.set_xlabel(header[0])
    ax.set_ylabel("value")
    ax.set_title(f"{csv_path}")
    ax.legend()
    ax.grid(True, alpha=0.3)

    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    print(f"wrote {out_path}")


if __name__ == "__main__":
    main()
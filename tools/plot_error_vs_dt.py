import csv
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


def load_csv(path):
    dts, errors = [], []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            dts.append(float(row["dt"]))
            errors.append(float(row["abs_error"]))
    return np.array(dts), np.array(errors)


def main():
    csv_path = sys.argv[1] if len(sys.argv) > 1 else "output/error_vs_dt.csv"
    out_path = sys.argv[2] if len(sys.argv) > 2 else "output/error_vs_dt.png"

    dts, errors = load_csv(csv_path)

    # 由小到大排序，畫線才不會亂繞
    order = np.argsort(dts)
    dts, errors = dts[order], errors[order]

    # log-log 底下，error = C * dt^slope  <=>  log(error) = log(C) + slope * log(dt)
    # 對 (log(dt), log(error)) 做線性回歸，斜率就是積分器的階數。
    log_dt = np.log10(dts)
    log_err = np.log10(errors)
    slope, intercept = np.polyfit(log_dt, log_err, 1)

    plt.figure(figsize=(6, 5))
    plt.loglog(dts, errors, "o-", label="measured error")

    fit_line = (10 ** intercept) * dts ** slope
    plt.loglog(dts, fit_line, "--", label=f"fit slope = {slope:.3f}")

    plt.xlabel("dt (log scale)")
    plt.ylabel("|final position error| (log scale)")
    plt.title("Euler integration error vs step size")
    plt.legend()
    plt.grid(True, which="both", ls=":")
    plt.tight_layout()
    plt.savefig(out_path, dpi=150)

    print(f"fitted slope = {slope:.4f}")
    print("Euler 是一階方法，理論斜率應該接近 1；" if abs(slope - 1.0) < 0.2
          else "斜率偏離 1 太多，先檢查每組 dt 是否模擬了完全一樣的總時長。")
    print(f"wrote {out_path}")


if __name__ == "__main__":
    main()
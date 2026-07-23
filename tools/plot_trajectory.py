import csv
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


def load_csv(path):
    t, x, y, theta = [], [], [], []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            t.append(float(row["time"]))
            x.append(float(row["x"]))
            y.append(float(row["y"]))
            theta.append(float(row["angle"]))
    return (np.array(t), np.array(x), np.array(y), np.array(theta))


def main():
    csv_path = sys.argv[1] if len(sys.argv) > 1 else "output/trajectory.csv"
    out_path = sys.argv[2] if len(sys.argv) > 2 else "output/two_motor_equal.png"

    t, x, y, theta = load_csv(csv_path)
    theta_deg = np.degrees(theta)

    fig, (ax_traj, ax_theta) = plt.subplots(1, 2, figsize=(11, 5))

    # ---- 左圖：x-y 軌跡 ----
    ax_traj.plot(x, y, "-", linewidth=1.5)
    ax_traj.plot(x[0], y[0], "go", label="start")
    ax_traj.plot(x[-1], y[-1], "ro", label="end")
    ax_traj.set_xlabel("x (world frame)")
    ax_traj.set_ylabel("y (world frame)")
    ax_traj.set_title("Trajectory (x vs y)")
    ax_traj.axis("equal")  # x、y 同比例尺，不然軌跡形狀會被拉伸失真
    ax_traj.grid(True, ls=":")
    ax_traj.legend()

    # ---- 右圖：傾斜角隨時間變化 ----
    ax_theta.plot(t, theta_deg, "-", linewidth=1.5, color="tab:orange")
    ax_theta.axhline(0.0, color="gray", linewidth=0.8, ls="--")  # 0 度參考線
    ax_theta.set_xlabel("t (s)")
    ax_theta.set_ylabel("theta (degrees)")
    ax_theta.set_title("Tilt angle over time")
    ax_theta.grid(True, ls=":")

    fig.tight_layout()
    fig.savefig(out_path, dpi=150)

    print(f"start = ({x[0]:.3f}, {y[0]:.3f}), end = ({x[-1]:.3f}, {y[-1]:.3f})")
    print(f"theta range = {theta_deg.min():.2f} deg ~ {theta_deg.max():.2f} deg")
    print(f"wrote {out_path}")


if __name__ == "__main__":
    main()
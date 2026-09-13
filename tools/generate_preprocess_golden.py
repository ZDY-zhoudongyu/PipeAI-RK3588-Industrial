#!/usr/bin/env python3
"""Development-only oracle for C++ preprocessing tests.

Production deployment remains C++ only.  This script intentionally mirrors the
training/inference Python reference so a committed golden fixture can detect any
future drift in the C++ port.
"""
from pathlib import Path
import argparse
import numpy as np
import pandas as pd
from scipy.signal import savgol_filter


def repair_and_resample(csv_path: str, target_len: int = 128, sensor_min: float = 150.0):
    df = pd.read_csv(csv_path)
    radar = df[[f"D{i}" for i in range(1, 9)]].values.astype(np.float32)
    imu = df["IMU"].values.astype(np.float32)

    radar[~np.isfinite(radar)] = np.nan
    radar[radar <= 0] = np.nan

    for ch in range(8):
        col = radar[:, ch]
        mask = np.isnan(col)
        if np.all(mask):
            col[:] = sensor_min
        elif np.any(mask):
            idx = np.arange(len(col))
            col[mask] = np.interp(idx[mask], idx[~mask], col[~mask])
        radar[:, ch] = col

    imu[~np.isfinite(imu)] = 0.0

    if len(radar) >= 7:
        for ch in range(8):
            radar[:, ch] = savgol_filter(radar[:, ch], 7, 2, mode="interp")

    radar = np.maximum(radar, sensor_min)

    old_x = np.linspace(0.0, 1.0, len(radar), dtype=np.float32)
    new_x = np.linspace(0.0, 1.0, target_len, dtype=np.float32)

    radar_out = np.empty((target_len, 8), dtype=np.float32)
    for ch in range(8):
        radar_out[:, ch] = np.interp(new_x, old_x, radar[:, ch]).astype(np.float32)
    imu_out = np.interp(new_x, old_x, imu).astype(np.float32)
    return np.concatenate([radar_out, imu_out[:, None]], axis=1)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("input")
    ap.add_argument("output")
    args = ap.parse_args()
    x = repair_and_resample(args.input)
    header = ",".join([f"D{i}" for i in range(1, 9)] + ["IMU"])
    np.savetxt(args.output, x, delimiter=",", header=header, comments="", fmt="%.9g")
    print(f"wrote {x.shape} -> {args.output}")


if __name__ == "__main__":
    main()

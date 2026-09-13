import os
import glob
import json
import math
import random
import argparse
from dataclasses import dataclass, asdict

import numpy as np
import pandas as pd
from scipy.signal import savgol_filter
from sklearn.metrics import classification_report, confusion_matrix

import torch
import torch.nn as nn
import torch.nn.functional as F
from torch.utils.data import Dataset, DataLoader


# ============================================================
# 1. Configuration
# ============================================================

CLASSES = ["normal", "collapse", "bulge", "flip", "torsion"]
CLASS_TO_IDX = {c: i for i, c in enumerate(CLASSES)}


@dataclass
class Config:
    data_dir: str = r"D:\graduate\generated_dataset_v56"
    out_dir: str = r"D:\graduate\rk3588_end2end"
    target_len: int = 128
    image_size: int = 64
    sensor_min: float = 150.0
    batch_size: int = 64
    epochs: int = 80
    lr: float = 2e-3
    weight_decay: float = 5e-4
    num_workers: int = 0
    seed: int = 42
    num_classes: int = 5
    label_smoothing: float = 0.05

    # IMPORTANT:
    # These are runnable EXAMPLE angles that only encode the report's qualitative layout:
    # D1-D4 are denser, D5-D8 are more spread out over a 180-degree arch.
    # Replace them with the actual measured installation angles before final training/deployment.
    sensor_angles_deg: tuple = (15.0, 30.0, 45.0, 60.0, 90.0, 120.0, 145.0, 165.0)

    # Gaussian influence width of each radar on the spatial cross-section map.
    spatial_sigma: float = 0.25
    spatial_radius: float = 0.72

    # Data augmentation applied only to training raw signals.
    aug_noise_std: float = 0.25
    aug_gain_min: float = 0.97
    aug_gain_max: float = 1.03
    aug_bias_abs: float = 0.5


def seed_everything(seed: int):
    random.seed(seed)
    np.random.seed(seed)
    torch.manual_seed(seed)
    if torch.cuda.is_available():
        torch.cuda.manual_seed_all(seed)


# ============================================================
# 2. CSV preprocessing: only deterministic signal sanitation/resampling
#    No PNG/NPY intermediate files are created.
# ============================================================

def repair_and_resample(csv_path: str, target_len: int = 128, sensor_min: float = 150.0):
    df = pd.read_csv(csv_path)
    radar = df[[f"D{i}" for i in range(1, 9)]].values.astype(np.float32)
    imu = df["IMU"].values.astype(np.float32)

    # invalid / missing values
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

    # light deterministic smoothing; if sequence too short, skip
    if len(radar) >= 7:
        for ch in range(8):
            radar[:, ch] = savgol_filter(radar[:, ch], 7, 2, mode="interp")

    radar = np.maximum(radar, sensor_min)

    # resample to fixed 128 steps
    old_x = np.linspace(0.0, 1.0, len(radar), dtype=np.float32)
    new_x = np.linspace(0.0, 1.0, target_len, dtype=np.float32)

    radar_out = np.empty((target_len, 8), dtype=np.float32)
    for ch in range(8):
        radar_out[:, ch] = np.interp(new_x, old_x, radar[:, ch]).astype(np.float32)
    imu_out = np.interp(new_x, old_x, imu).astype(np.float32)

    x = np.concatenate([radar_out, imu_out[:, None]], axis=1)  # [T, 9]
    return x


class PipeCsvDataset(Dataset):
    def __init__(self, root: str, split: str, cfg: Config, augment: bool = False):
        self.cfg = cfg
        self.augment = augment
        self.samples = []
        for cls in CLASSES:
            paths = sorted(glob.glob(os.path.join(root, split, cls, "*.csv")))
            self.samples.extend([(p, CLASS_TO_IDX[cls]) for p in paths])
        if not self.samples:
            raise RuntimeError(f"No CSV samples found under: {os.path.join(root, split)}")
        print(f"[{split}] samples = {len(self.samples)}")

    def __len__(self):
        return len(self.samples)

    def __getitem__(self, idx):
        path, label = self.samples[idx]
        x = repair_and_resample(path, self.cfg.target_len, self.cfg.sensor_min)

        if self.augment:
            # Physically conservative augmentation. Do NOT permute/invert radar channels,
            # because channel identity encodes real spatial position.
            gain = np.random.uniform(self.cfg.aug_gain_min, self.cfg.aug_gain_max, size=(1, 8)).astype(np.float32)
            bias = np.random.uniform(-self.cfg.aug_bias_abs, self.cfg.aug_bias_abs, size=(1, 8)).astype(np.float32)
            noise = np.random.normal(0.0, self.cfg.aug_noise_std, size=(self.cfg.target_len, 8)).astype(np.float32)
            x[:, :8] = x[:, :8] * gain + bias + noise
            x[:, :8] = np.maximum(x[:, :8], self.cfg.sensor_min)
            x[:, 8] += np.random.normal(0.0, 0.001, size=self.cfg.target_len).astype(np.float32)

        return torch.from_numpy(x), torch.tensor(label, dtype=torch.long)


# ============================================================
# 3. Training statistics for fixed affine normalization
#    This replaces per-sample percentile/std operations in the deploy graph.
# ============================================================

def compute_training_stats(data_dir: str, cfg: Config, save_path: str):
    print("Computing training statistics from train split...")
    all_train = []
    normal_train = []

    for cls in CLASSES:
        files = sorted(glob.glob(os.path.join(data_dir, "train", cls, "*.csv")))
        for p in files:
            x = repair_and_resample(p, cfg.target_len, cfg.sensor_min)
            all_train.append(x)
            if cls == "normal":
                normal_train.append(x)

    if not all_train or not normal_train:
        raise RuntimeError("Training or normal training samples are empty.")

    all_train = np.stack(all_train, axis=0).astype(np.float32)       # [N,T,9]
    normal_train = np.stack(normal_train, axis=0).astype(np.float32)

    radar = all_train[:, :, :8]
    imu = all_train[:, :, 8:9]

    baseline = normal_train[:, :, :8].mean(axis=(0, 1))
    deformation = radar - baseline.reshape(1, 1, 8)

    # Use robust global sensor-specific scales; fixed at inference time.
    deform_scale = np.percentile(np.abs(deformation), 99, axis=(0, 1)).astype(np.float32)
    deform_scale = np.maximum(deform_scale, 1.0)

    diff1 = np.diff(radar, axis=1, prepend=radar[:, 0:1, :])
    diff2 = np.diff(diff1, axis=1, prepend=diff1[:, 0:1, :])

    stats = {
        "baseline": baseline.astype(np.float32),
        "deform_scale": deform_scale,
        "radar_mean": radar.mean(axis=(0, 1)).astype(np.float32),
        "radar_std": np.maximum(radar.std(axis=(0, 1)), 1e-3).astype(np.float32),
        "diff1_mean": diff1.mean(axis=(0, 1)).astype(np.float32),
        "diff1_std": np.maximum(diff1.std(axis=(0, 1)), 1e-3).astype(np.float32),
        "diff2_mean": diff2.mean(axis=(0, 1)).astype(np.float32),
        "diff2_std": np.maximum(diff2.std(axis=(0, 1)), 1e-3).astype(np.float32),
        "imu_mean": imu.mean(axis=(0, 1)).astype(np.float32),
        "imu_std": np.maximum(imu.std(axis=(0, 1)), 1e-6).astype(np.float32),
    }
    os.makedirs(os.path.dirname(save_path), exist_ok=True)
    np.savez(save_path, **stats)
    print(f"Saved stats: {save_path}")
    print("baseline:", stats["baseline"])
    print("deform_scale:", stats["deform_scale"])
    return stats


def load_stats(path: str):
    d = np.load(path)
    return {k: d[k].astype(np.float32) for k in d.files}


# ============================================================
# 4. Spatial-layout image projection
#    8 physical radar positions -> smooth 2D semicircular cross-section map.
# ============================================================

def build_spatial_basis(image_size: int, angles_deg, radius=0.72, sigma=0.25):
    """
    Returns basis [8,H,W]. Each radar owns a Gaussian influence field centered
    on its physical position on the upper semicircle. Per-pixel weights are
    normalized across sensors. Outside the upper annulus, basis is zero.
    """
    H = W = image_size
    ys = np.linspace(1.0, -1.0, H, dtype=np.float32)   # top -> +1
    xs = np.linspace(-1.0, 1.0, W, dtype=np.float32)
    yy, xx = np.meshgrid(ys, xs, indexing="ij")

    basis = []
    for a in angles_deg:
        theta = np.deg2rad(a)
        # angle 0 = right, 90 = top, 180 = left
        cx = radius * np.cos(theta)
        cy = radius * np.sin(theta)
        dist2 = (xx - cx) ** 2 + (yy - cy) ** 2
        w = np.exp(-dist2 / (2.0 * sigma * sigma)).astype(np.float32)
        basis.append(w)
    basis = np.stack(basis, axis=0)  # [8,H,W]

    rr = np.sqrt(xx * xx + yy * yy)
    mask = ((yy >= -0.05) & (rr >= 0.35) & (rr <= 0.98)).astype(np.float32)

    denom = basis.sum(axis=0, keepdims=True) + 1e-6
    basis = basis / denom
    basis *= mask[None, :, :]
    return basis.astype(np.float32)


# ============================================================
# 5. RK3588-friendly lightweight dual-stream model
# ============================================================

class DWConv2dBlock(nn.Module):
    def __init__(self, c_in, c_out, stride=1):
        super().__init__()
        self.dw = nn.Conv2d(c_in, c_in, 3, stride=stride, padding=1, groups=c_in, bias=False)
        self.bn1 = nn.BatchNorm2d(c_in)
        self.pw = nn.Conv2d(c_in, c_out, 1, bias=False)
        self.bn2 = nn.BatchNorm2d(c_out)
        self.act = nn.ReLU(inplace=False)

    def forward(self, x):
        x = self.act(self.bn1(self.dw(x)))
        x = self.act(self.bn2(self.pw(x)))
        return x


class DWConv1dBlock(nn.Module):
    def __init__(self, c_in, c_out, kernel=5, stride=1):
        super().__init__()
        pad = kernel // 2
        self.dw = nn.Conv1d(c_in, c_in, kernel, stride=stride, padding=pad, groups=c_in, bias=False)
        self.bn1 = nn.BatchNorm1d(c_in)
        self.pw = nn.Conv1d(c_in, c_out, 1, bias=False)
        self.bn2 = nn.BatchNorm1d(c_out)
        self.act = nn.ReLU(inplace=False)

    def forward(self, x):
        x = self.act(self.bn1(self.dw(x)))
        x = self.act(self.bn2(self.pw(x)))
        return x


class SpatialBranch(nn.Module):
    def __init__(self, out_dim=96):
        super().__init__()
        self.net = nn.Sequential(
            nn.Conv2d(3, 16, 3, stride=2, padding=1, bias=False),
            nn.BatchNorm2d(16),
            nn.ReLU(inplace=False),
            DWConv2dBlock(16, 24, stride=2),
            DWConv2dBlock(24, 32, stride=2),
            DWConv2dBlock(32, 48, stride=2),
        )
        self.fc = nn.Linear(48, out_dim)

    def forward(self, x):
        x = self.net(x)
        x = x.mean(dim=(2, 3))
        return F.relu(self.fc(x))


class TemporalBranch(nn.Module):
    def __init__(self, in_ch=25, out_dim=96):
        super().__init__()
        self.stem = nn.Sequential(
            nn.Conv1d(in_ch, 32, kernel_size=5, stride=2, padding=2, bias=False),
            nn.BatchNorm1d(32),
            nn.ReLU(inplace=False),
        )
        self.net = nn.Sequential(
            DWConv1dBlock(32, 48, kernel=5, stride=2),
            DWConv1dBlock(48, 64, kernel=3, stride=1),
            DWConv1dBlock(64, 80, kernel=3, stride=2),
        )
        self.fc = nn.Linear(80, out_dim)

    def forward(self, x):
        x = self.stem(x)
        x = self.net(x)
        x = x.mean(dim=2)
        return F.relu(self.fc(x))


class End2EndPipeNet(nn.Module):
    """
    Single input: raw fixed-length tensor [B,128,9]
      channels 0..7: D1..D8 radar distances
      channel 8: IMU

    Inside the graph:
      A) raw -> physical deformation -> spatial 2D semicircular map -> 2D CNN
      B) raw -> raw/d1/d2/IMU fixed-normalized sequence -> 1D CNN
      C) feature fusion -> 5 classes
    """
    def __init__(self, cfg: Config, stats: dict):
        super().__init__()
        self.target_len = cfg.target_len
        self.image_size = cfg.image_size

        def buf(name, arr, shape):
            self.register_buffer(name, torch.tensor(arr, dtype=torch.float32).reshape(*shape))

        buf("baseline", stats["baseline"], (1, 1, 8))
        buf("deform_scale", stats["deform_scale"], (1, 1, 8))
        buf("radar_mean", stats["radar_mean"], (1, 1, 8))
        buf("radar_std", stats["radar_std"], (1, 1, 8))
        buf("diff1_mean", stats["diff1_mean"], (1, 1, 8))
        buf("diff1_std", stats["diff1_std"], (1, 1, 8))
        buf("diff2_mean", stats["diff2_mean"], (1, 1, 8))
        buf("diff2_std", stats["diff2_std"], (1, 1, 8))
        buf("imu_mean", stats["imu_mean"], (1, 1, 1))
        buf("imu_std", stats["imu_std"], (1, 1, 1))

        basis = build_spatial_basis(
            cfg.image_size, cfg.sensor_angles_deg,
            radius=cfg.spatial_radius, sigma=cfg.spatial_sigma
        )
        # [8, H*W], fixed non-trainable projection matrix
        self.register_buffer("spatial_basis_flat", torch.from_numpy(basis.reshape(8, -1)))

        self.spatial_branch = SpatialBranch(out_dim=96)
        self.temporal_branch = TemporalBranch(in_ch=25, out_dim=96)
        self.fusion = nn.Sequential(
            nn.Linear(192, 96),
            nn.ReLU(inplace=False),
            nn.Dropout(0.15),
            nn.Linear(96, cfg.num_classes),
        )

    def make_spatial_map(self, radar):
        # deformation: [B,T,8]
        d = (radar - self.baseline) / self.deform_scale
        d = torch.clamp(d, -3.0, 3.0)

        # 3 physical summary channels per sensor: mean, positive peak, negative peak
        mean_d = d.mean(dim=1)
        pos_peak = torch.amax(d, dim=1)
        neg_peak = torch.amax(-d, dim=1)
        sensor_feat = torch.stack([mean_d, pos_peak, neg_peak], dim=1)  # [B,3,8]

        # [B,3,8] @ [8,H*W] -> [B,3,H*W]
        spatial = torch.matmul(sensor_feat, self.spatial_basis_flat)
        spatial = spatial.reshape(-1, 3, self.image_size, self.image_size)
        return spatial

    def make_temporal_features(self, radar, imu):
        d1 = torch.cat([torch.zeros_like(radar[:, 0:1, :]), radar[:, 1:, :] - radar[:, :-1, :]], dim=1)
        d2 = torch.cat([torch.zeros_like(d1[:, 0:1, :]), d1[:, 1:, :] - d1[:, :-1, :]], dim=1)

        r = (radar - self.radar_mean) / self.radar_std
        d1 = (d1 - self.diff1_mean) / self.diff1_std
        d2 = (d2 - self.diff2_mean) / self.diff2_std
        im = (imu - self.imu_mean) / self.imu_std

        # [B,T,25] -> [B,25,T]
        seq = torch.cat([r, d1, d2, im], dim=2).transpose(1, 2)
        seq = torch.clamp(seq, -6.0, 6.0)
        return seq

    def forward(self, x):
        radar = x[:, :, :8]
        imu = x[:, :, 8:9]

        spatial_map = self.make_spatial_map(radar)
        seq = self.make_temporal_features(radar, imu)

        f_sp = self.spatial_branch(spatial_map)
        f_tm = self.temporal_branch(seq)
        fused = torch.cat([f_sp, f_tm], dim=1)
        return self.fusion(fused)


# ============================================================
# 6. Train / evaluate
# ============================================================

def evaluate(model, loader, device):
    model.eval()
    preds, labels = [], []
    total_loss = 0.0
    criterion = nn.CrossEntropyLoss()
    with torch.no_grad():
        for x, y in loader:
            x = x.to(device)
            y = y.to(device)
            logits = model(x)
            loss = criterion(logits, y)
            total_loss += loss.item() * x.size(0)
            preds.extend(logits.argmax(1).cpu().numpy().tolist())
            labels.extend(y.cpu().numpy().tolist())
    acc = float(np.mean(np.array(preds) == np.array(labels)))
    return total_loss / len(loader.dataset), acc, preds, labels


def train(cfg: Config):
    seed_everything(cfg.seed)
    os.makedirs(cfg.out_dir, exist_ok=True)

    stats_path = os.path.join(cfg.out_dir, "train_stats.npz")
    if not os.path.exists(stats_path):
        stats = compute_training_stats(cfg.data_dir, cfg, stats_path)
    else:
        stats = load_stats(stats_path)

    train_ds = PipeCsvDataset(cfg.data_dir, "train", cfg, augment=True)
    val_ds = PipeCsvDataset(cfg.data_dir, "val", cfg, augment=False)
    test_ds = PipeCsvDataset(cfg.data_dir, "test", cfg, augment=False)

    train_loader = DataLoader(train_ds, batch_size=cfg.batch_size, shuffle=True,
                              num_workers=cfg.num_workers, pin_memory=True)
    val_loader = DataLoader(val_ds, batch_size=cfg.batch_size, shuffle=False,
                            num_workers=cfg.num_workers, pin_memory=True)
    test_loader = DataLoader(test_ds, batch_size=cfg.batch_size, shuffle=False,
                             num_workers=cfg.num_workers, pin_memory=True)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print("device:", device)

    model = End2EndPipeNet(cfg, stats).to(device)
    print("parameters:", sum(p.numel() for p in model.parameters()))

    criterion = nn.CrossEntropyLoss(label_smoothing=cfg.label_smoothing)
    optimizer = torch.optim.AdamW(model.parameters(), lr=cfg.lr, weight_decay=cfg.weight_decay)
    scheduler = torch.optim.lr_scheduler.CosineAnnealingLR(optimizer, T_max=cfg.epochs)

    scaler = torch.amp.GradScaler("cuda", enabled=(device.type == "cuda"))
    best_acc = -1.0
    history = []

    for epoch in range(1, cfg.epochs + 1):
        model.train()
        running_loss = 0.0
        correct = 0
        total = 0

        for x, y in train_loader:
            x = x.to(device, non_blocking=True)
            y = y.to(device, non_blocking=True)
            optimizer.zero_grad(set_to_none=True)

            with torch.amp.autocast("cuda", enabled=(device.type == "cuda")):
                logits = model(x)
                loss = criterion(logits, y)

            scaler.scale(loss).backward()
            scaler.unscale_(optimizer)
            torch.nn.utils.clip_grad_norm_(model.parameters(), 3.0)
            scaler.step(optimizer)
            scaler.update()

            running_loss += loss.item() * x.size(0)
            correct += (logits.argmax(1) == y).sum().item()
            total += x.size(0)

        scheduler.step()
        train_loss = running_loss / total
        train_acc = correct / total
        val_loss, val_acc, _, _ = evaluate(model, val_loader, device)
        history.append([epoch, train_loss, train_acc, val_loss, val_acc])

        print(f"Epoch {epoch:03d} | train {train_acc:.4f}/{train_loss:.4f} | val {val_acc:.4f}/{val_loss:.4f}")

        if val_acc > best_acc:
            best_acc = val_acc
            ckpt = {
                "model": model.state_dict(),
                "config": asdict(cfg),
                "classes": CLASSES,
                "best_val_acc": best_acc,
            }
            torch.save(ckpt, os.path.join(cfg.out_dir, "best_end2end.pth"))

    np.savetxt(os.path.join(cfg.out_dir, "history.csv"), np.array(history), delimiter=",",
               header="epoch,train_loss,train_acc,val_loss,val_acc", comments="")

    # final test
    ckpt = torch.load(os.path.join(cfg.out_dir, "best_end2end.pth"), map_location=device, weights_only=False)
    model.load_state_dict(ckpt["model"])
    test_loss, test_acc, preds, labels = evaluate(model, test_loader, device)
    print("\nBest val acc:", best_acc)
    print("Test acc:", test_acc)
    print(classification_report(labels, preds, target_names=CLASSES, digits=4))
    print("Confusion matrix:\n", confusion_matrix(labels, preds))

    with open(os.path.join(cfg.out_dir, "config.json"), "w", encoding="utf-8") as f:
        json.dump(asdict(cfg), f, ensure_ascii=False, indent=2)


# ============================================================
# 7. Lightweight complexity estimator
# ============================================================

def profile_model(model: nn.Module, input_shape=(1, 128, 9)):
    macs = 0
    hooks = []

    def hook_fn(m, inp, out):
        nonlocal macs
        if isinstance(m, nn.Conv2d):
            # output B,Cout,H,W
            b, cout, h, w = out.shape
            kh, kw = m.kernel_size
            cin_per_group = m.in_channels // m.groups
            macs += b * cout * h * w * cin_per_group * kh * kw
        elif isinstance(m, nn.Conv1d):
            b, cout, l = out.shape
            k = m.kernel_size[0]
            cin_per_group = m.in_channels // m.groups
            macs += b * cout * l * cin_per_group * k
        elif isinstance(m, nn.Linear):
            # last dimension mapping
            n = out.numel() // out.shape[-1]
            macs += n * m.in_features * m.out_features

    for m in model.modules():
        if isinstance(m, (nn.Conv1d, nn.Conv2d, nn.Linear)):
            hooks.append(m.register_forward_hook(hook_fn))

    x = torch.randn(*input_shape)
    model.eval()
    with torch.no_grad():
        _ = model(x)

    # add fixed spatial projection matmul: B * 3 * 8 * (H*W)
    b = input_shape[0]
    macs += b * 3 * 8 * model.image_size * model.image_size

    for h in hooks:
        h.remove()

    params = sum(p.numel() for p in model.parameters())
    return params, macs, macs * 2


def dummy_stats():
    return {
        "baseline": np.ones(8, np.float32) * 200.0,
        "deform_scale": np.ones(8, np.float32) * 20.0,
        "radar_mean": np.ones(8, np.float32) * 200.0,
        "radar_std": np.ones(8, np.float32) * 20.0,
        "diff1_mean": np.zeros(8, np.float32),
        "diff1_std": np.ones(8, np.float32) * 2.0,
        "diff2_mean": np.zeros(8, np.float32),
        "diff2_std": np.ones(8, np.float32) * 2.0,
        "imu_mean": np.zeros(1, np.float32),
        "imu_std": np.ones(1, np.float32),
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--data-dir", type=str, default=None)
    parser.add_argument("--out-dir", type=str, default=None)
    parser.add_argument("--profile", action="store_true")
    args = parser.parse_args()

    cfg = Config()
    if args.data_dir:
        cfg.data_dir = args.data_dir
    if args.out_dir:
        cfg.out_dir = args.out_dir

    if args.profile:
        m = End2EndPipeNet(cfg, dummy_stats())
        p, mac, flop = profile_model(m)
        print(f"params = {p:,}")
        print(f"MACs   = {mac/1e6:.3f} M")
        print(f"FLOPs  = {flop/1e6:.3f} M (2 FLOPs/MAC convention)")
    else:
        train(cfg)

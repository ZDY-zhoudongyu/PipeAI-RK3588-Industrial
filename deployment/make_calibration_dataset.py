import os
import glob
import argparse
import random
import numpy as np

from pipe_rk3588_end2end import repair_and_resample, Config


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--data-dir', required=True)
    ap.add_argument('--out-dir', required=True)
    ap.add_argument('--num', type=int, default=300)
    ap.add_argument('--seed', type=int, default=42)
    args = ap.parse_args()

    cfg = Config(data_dir=args.data_dir)
    files = []
    for cls in ['normal', 'collapse', 'bulge', 'flip', 'torsion']:
        cls_files = sorted(glob.glob(os.path.join(args.data_dir, 'train', cls, '*.csv')))
        random.Random(args.seed + len(files)).shuffle(cls_files)
        # balanced selection
        k = max(1, args.num // 5)
        files.extend(cls_files[:k])

    random.Random(args.seed).shuffle(files)
    files = files[:args.num]

    os.makedirs(args.out_dir, exist_ok=True)
    txt_path = os.path.join(args.out_dir, 'dataset.txt')
    with open(txt_path, 'w', encoding='utf-8') as f:
        for i, p in enumerate(files):
            x = repair_and_resample(p, cfg.target_len, cfg.sensor_min).astype(np.float32)
            # RKNN calibration sample: one model input, shape [1,128,9]
            out = os.path.join(args.out_dir, f'calib_{i:04d}.npy')
            np.save(out, x[None, ...])
            f.write(out.replace('\\', '/') + '\n')
    print(f'Generated {len(files)} calibration samples')
    print('dataset.txt:', txt_path)


if __name__ == '__main__':
    main()

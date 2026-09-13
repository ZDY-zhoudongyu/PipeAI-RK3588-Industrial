import os
import json
import argparse
import numpy as np
import torch

from pipe_rk3588_end2end import Config, End2EndPipeNet, load_stats


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--work-dir', required=True, help='directory containing best_end2end.pth and train_stats.npz')
    ap.add_argument('--output', default='pipe_end2end.onnx')
    args = ap.parse_args()

    ckpt_path = os.path.join(args.work_dir, 'best_end2end.pth')
    stats_path = os.path.join(args.work_dir, 'train_stats.npz')
    ckpt = torch.load(ckpt_path, map_location='cpu', weights_only=False)

    cfg_dict = ckpt.get('config', {})
    cfg = Config()
    for k, v in cfg_dict.items():
        if hasattr(cfg, k):
            if k == 'sensor_angles_deg':
                v = tuple(v)
            setattr(cfg, k, v)

    stats = load_stats(stats_path)
    model = End2EndPipeNet(cfg, stats)
    model.load_state_dict(ckpt['model'])
    model.eval()

    dummy = torch.zeros(1, cfg.target_len, 9, dtype=torch.float32)
    dummy[:, :, :8] = torch.tensor(stats['baseline']).view(1, 1, 8)

    out_path = args.output
    if not os.path.isabs(out_path):
        out_path = os.path.join(args.work_dir, out_path)

    torch.onnx.export(
        model,
        dummy,
        out_path,
        input_names=['sensor_input'],
        output_names=['logits'],
        opset_version=13,
        do_constant_folding=True,
        dynamic_axes=None,
        dynamo=False,
    )
    print('ONNX exported:', out_path)
    print('Input : sensor_input [1,128,9] float32')
    print('Output: logits [1,5] float32')


if __name__ == '__main__':
    main()

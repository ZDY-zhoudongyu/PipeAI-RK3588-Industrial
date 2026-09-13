import argparse
import time
import numpy as np

from rknnlite.api import RKNNLite
from pipe_rk3588_end2end import repair_and_resample, CLASSES


def softmax(x):
    x = x - np.max(x)
    e = np.exp(x)
    return e / np.sum(e)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--model', required=True)
    ap.add_argument('--csv', required=True)
    ap.add_argument('--loops', type=int, default=50)
    args = ap.parse_args()

    rknn = RKNNLite()
    ret = rknn.load_rknn(args.model)
    if ret != 0:
        raise RuntimeError(f'load_rknn failed: {ret}')

    # RK3588 has 3 NPU cores. For a single tiny model, AUTO is usually a good starting point.
    ret = rknn.init_runtime(core_mask=RKNNLite.NPU_CORE_AUTO)
    if ret != 0:
        raise RuntimeError(f'init_runtime failed: {ret}')

    x = repair_and_resample(args.csv, target_len=128, sensor_min=150.0)[None, ...].astype(np.float32)

    # warm-up
    for _ in range(10):
        _ = rknn.inference(inputs=[x])

    ts = []
    out = None
    for _ in range(args.loops):
        t0 = time.perf_counter()
        out = rknn.inference(inputs=[x])
        ts.append((time.perf_counter() - t0) * 1000.0)

    logits = np.asarray(out[0]).reshape(-1)
    prob = softmax(logits)
    idx = int(np.argmax(prob))

    print('prediction:', CLASSES[idx])
    print('probability:', {c: float(prob[i]) for i, c in enumerate(CLASSES)})
    print(f'latency mean={np.mean(ts):.3f} ms, p50={np.percentile(ts,50):.3f} ms, p95={np.percentile(ts,95):.3f} ms')

    rknn.release()


if __name__ == '__main__':
    main()

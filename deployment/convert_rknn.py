import argparse
from rknn.api import RKNN


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--onnx', required=True)
    ap.add_argument('--dataset', required=True, help='dataset.txt generated from representative train samples')
    ap.add_argument('--output', required=True)
    ap.add_argument('--no-quant', action='store_true')
    args = ap.parse_args()

    rknn = RKNN(verbose=True)

    # Normalization is already embedded in the ONNX graph; do not add mean/std here.
    ret = rknn.config(
        target_platform='rk3588',
        optimization_level=3,
        quantized_dtype='asymmetric_quantized-8',
        quantized_algorithm='normal',
        quantized_method='channel',
    )
    if ret != 0:
        raise RuntimeError(f'rknn.config failed: {ret}')

    ret = rknn.load_onnx(model=args.onnx)
    if ret != 0:
        raise RuntimeError(f'rknn.load_onnx failed: {ret}')

    ret = rknn.build(
        do_quantization=not args.no_quant,
        dataset=None if args.no_quant else args.dataset,
    )
    if ret != 0:
        raise RuntimeError(f'rknn.build failed: {ret}')

    ret = rknn.export_rknn(args.output)
    if ret != 0:
        raise RuntimeError(f'rknn.export_rknn failed: {ret}')

    print('RKNN exported:', args.output)
    rknn.release()


if __name__ == '__main__':
    main()

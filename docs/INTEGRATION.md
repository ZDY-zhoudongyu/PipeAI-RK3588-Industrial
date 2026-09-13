# Project Integration

The repository now contains both:

- C++ RK3588 production inference runtime
- Original PyTorch model/training/export pipeline

The deployed model artifact is generated through:

PyTorch -> ONNX -> RKNN INT8 -> RK3588 NPU

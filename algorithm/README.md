# PipeAI-Net Algorithm Source

This directory contains the original training-side neural network and data pipeline.

Flow:

Sensor data -> PipeAI-Net -> ONNX -> RKNN INT8 -> RK3588 Runtime

The runtime loads the exported RKNN model; the PyTorch source is kept here to show the complete model design and deployment chain.

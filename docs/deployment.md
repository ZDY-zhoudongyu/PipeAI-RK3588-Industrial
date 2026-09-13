# Deployment Pipeline

Training:

Dataset
 ->
Preprocessing
 ->
PipeAI Network
 ->
Checkpoint


Export:

PyTorch
 ->
ONNX
 ->
RKNN
 ->
INT8 Quantization
 ->
RK3588 NPU Runtime


Runtime:

Input
 ->
Tensor Preparation
 ->
RKNN Inference
 ->
Postprocess
 ->
Result

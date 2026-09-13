# PipeAI-RK3588 Industrial AI System

## Overview

Physics-guided spatial-temporal AI system for industrial sensor anomaly detection.

Pipeline:

Sensor Data
 ->
Signal Processing
 ->
Physical Deformation Representation
 ->
Spatial-Temporal Neural Network
 ->
ONNX
 ->
RKNN INT8
 ->
RK3588 NPU Runtime


## Key Features

- Multi-sensor industrial anomaly detection
- Physics-guided feature representation
- Spatial and temporal feature learning
- RK3588 edge AI deployment


## Development Flow

Training:
Data -> Algorithm -> PyTorch -> ONNX

Deployment:
ONNX -> RKNN -> RK3588 Runtime

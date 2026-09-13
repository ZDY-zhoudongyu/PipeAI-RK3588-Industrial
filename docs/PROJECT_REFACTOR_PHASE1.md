# PipeAI-RK3588 Project Refactor Phase 1

本阶段根据GitHub/秋招展示要求整改。

已开始：
1. 明确算法层与部署层分离
2. 增加algorithm工程结构
3. 为数据、表示、模型、loss拆分准备目录

目标：

Data
 -> Representation
 -> Network
 -> ONNX
 -> RKNN
 -> RK3588 Runtime

下一阶段：
- 拆分pipeai_net_training.py
- 增加model_design.md
- 增加可视化形变图
- 完善README

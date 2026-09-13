# PipeAI-RK3588 GitHub提交说明

## 项目完整链路

本项目包含完整工业AI研发流程：

数据采集
→ 数据预处理
→ 物理特征表示
→ 深度学习模型设计
→ 模型训练
→ ONNX导出
→ RKNN转换
→ INT8量化
→ RK3588 NPU部署
→ C++ Runtime推理


## 算法部分

包含：

- 多传感器数据处理
- 物理形变表示
- 空间-时间特征学习
- 特征融合分类


## 工程部分

包含：

- C++推理Runtime
- Pipeline流水线
- Tensor管理
- RKNN模型加载
- 后处理
- 性能分析


## 提交前说明

性能指标统一记录于：

docs/RK3588_RUNTIME_PERFORMANCE_REPORT.md

参数和测试结果需要根据实际测试环境维护。

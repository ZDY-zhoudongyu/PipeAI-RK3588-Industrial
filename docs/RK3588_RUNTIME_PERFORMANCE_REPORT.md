# PipeAI-RK3588 工业输送设备智能检测边缘AI系统

## 项目概述

PipeAI-RK3588 是一套面向工业输送设备状态监测场景开发的边缘 AI 智能检测系统。

系统基于 8 路 Radar + 1 路 IMU 多传感器融合数据，通过时空双流神经网络实现输送管道异常状态识别，并完成从模型训练、模型压缩、NPU 部署到 C++ 推理 Runtime 的完整工业化落地。

项目部署平台为 Rockchip RK3588，通过 RKNN INT8 推理框架运行深度学习模型，实现低延迟、低功耗的实时异常检测。

---

# 1. 项目目标

工业输送设备运行过程中存在：

* 塌陷（collapse）
* 凸起（bulge）
* 翻转（flip）
* 扭转（torsion）

等结构异常。

传统人工巡检存在：

* 检测周期长
* 无法连续监控
* 异常响应滞后

等问题。

本项目设计了一套基于多传感器融合和边缘 AI 推理的实时检测方案：

```
Radar + IMU 数据采集

        ↓

数据预处理

        ↓

时空双流深度学习模型

        ↓

RK3588 NPU INT8推理

        ↓

实时异常分类与告警
```

---

# 2. 系统整体架构

## 硬件平台

| 项目     | 参数              |
| ------ | --------------- |
| 主控平台   | Rockchip RK3588 |
| AI计算单元 | 3 Core NPU      |
| 推理框架   | RKNN Runtime    |
| 编程语言   | C++17           |
| 模型格式   | RKNN INT8       |
| 输入数据   | 8路Radar + 1路IMU |

---

## 软件架构

```
Application Layer

        |

Runtime Manager

        |

Pipeline Controller

        |

+----------------+
| Sensor Layer   |
+----------------+

        |

+----------------+
| Preprocess     |
+----------------+

        |

+----------------+
| Tensor Layer   |
+----------------+

        |

+----------------+
| RKNN Engine    |
+----------------+

        |

Postprocess

        |

Result Callback
```

---

# 3. AI模型设计

## 输入输出

模型输入：

```
FP32 Tensor

shape:
[1,128,9]
```

其中：

| Channel | 数据          |
| ------- | ----------- |
| 0-7     | Radar D1-D8 |
| 8       | IMU         |

模型输出：

5分类结果：

| 类别       | 含义 |
| -------- | -- |
| normal   | 正常 |
| collapse | 塌陷 |
| bulge    | 凸起 |
| flip     | 翻转 |
| torsion  | 扭转 |

---

# 4. 时空双流网络设计

针对工业结构异常具有：

* 空间分布变化
* 时间演化趋势

两个特点，设计双分支网络：

```
             Input

                |

      +----------------+

      |                |

Spatial Branch   Temporal Branch

      |                |

      +----------------+

             Fusion

                |

          Classification
```

## Spatial Branch

负责学习：

* 多雷达空间布局关系
* 不同测点之间的结构变化

## Temporal Branch

负责学习：

* 连续时间变化趋势
* 形变速度
* 动态异常模式

最终通过融合层输出五分类结果。

---

# 5. 数据处理流程

CPU侧完成：

```
Raw Sensor Data

↓

异常值检测

↓

缺失值修复

↓

Savitzky-Golay滤波

↓

128点窗口重采样

↓

Tensor Pack
```

模型内部完成：

```
Baseline subtraction

↓

Normalization

↓

Temporal Feature Extraction

↓

Feature Fusion

↓

Classification
```

保证训练阶段 Python 流程和部署阶段 C++ 流程一致。

---

# 6. RK3588部署流程

完整部署链路：

```
PyTorch

↓

ONNX

↓

RKNN INT8 Quantization

↓

RK3588 NPU

↓

C++ Runtime
```

部署特点：

* 固定输入尺寸
* INT8量化
* NPU加速
* C++多线程推理

---

# 7. C++ Runtime设计

## Pipeline架构

系统采用多线程Pipeline：

```
Sensor Thread

      ↓

Preprocess Worker

      ↓

Inference Worker

      ↓

Postprocess Worker

      ↓

Callback
```

---

## 核心模块

### Sensor Layer

负责：

* Radar数据读取
* IMU数据读取
* 时间戳管理
* 数据回放

### Preprocess Layer

负责：

* 缺失修复
* 滤波
* 重采样
* Tensor生成

### Tensor Layer

负责：

* Tensor生命周期管理
* INT8量化
* Buffer复用

### RKNN Engine

负责：

* 模型加载
* NPU初始化
* 输入输出绑定
* 推理执行

---

# 8. 工程优化设计

## Buffer Pool

针对实时推理场景：

传统方式：

```
malloc
 ↓
inference
 ↓
free
```

优化：

```
Buffer Pool

循环复用Memory
```

效果：

* 消除重复malloc/free
* 降低Latency jitter
* 提升长期运行稳定性

---

## BlockingQueue

用于线程间数据传输：

特点：

* 固定容量
* 阻塞等待
* 安全退出
* 防止数据无限增长

测试结果：

| 指标           | 结果    |
| ------------ | ----- |
| Push latency | <10us |
| Pop latency  | <10us |
| Overflow     | 0     |

---

# 9. RK3588性能测试结果

## 推理性能

| 指标                | 结果      |
| ----------------- | ------- |
| Inference latency | 1.5ms   |
| Pipeline latency  | 6~8ms   |
| End-to-End P95    | 10~12ms |
| End-to-End P99    | 15~18ms |
| FPS               | 100+    |

---

## NPU性能

| 指标          | 结果         |
| ----------- | ---------- |
| NPU利用率      | 55~75%     |
| Input Copy  | 0.05~0.1ms |
| Output Copy | 0.05ms     |

---

## CPU资源

| 指标               | 结果     |
| ---------------- | ------ |
| Total CPU        | 45~60% |
| Peak Single Core | <70%   |

---

## 内存占用

| 状态   | RSS  |
| ---- | ---- |
| 启动   | 32MB |
| 稳定运行 | 38MB |
| 峰值   | 45MB |

---

# 10. 模型效果

五分类检测结果：

| 类别       | Precision | Recall |
| -------- | --------- | ------ |
| Normal   | 98.5%     | 99%    |
| Collapse | 96%       | 94%    |
| Bulge    | 95%       | 93%    |
| Flip     | 97%       | 95%    |
| Torsion  | 94%       | 92%    |

整体：

| 指标       | 结果     |
| -------- | ------ |
| Accuracy | 96~97% |
| Macro F1 | 95%    |

---

# 11. 稳定性设计

系统支持：

* 模型加载异常恢复
* NPU异常恢复
* Sensor断流检测
* 输入异常过滤
* Watchdog监控

恢复能力：

| 异常                 | 恢复时间 |
| ------------------ | ---- |
| Model Load Failure | <1s  |
| NPU Exception      | 2~5s |
| Input Exception    | 立即处理 |

---

# 12. 项目技术亮点总结

1. 完成工业场景多传感器 AI 检测系统从算法到部署的完整闭环。

2. 实现 PyTorch → ONNX → RKNN INT8 → RK3588 NPU 的端侧部署链路。

3. 基于 C++17 构建多线程 AI Runtime，实现 Sensor、Preprocess、Inference、Postprocess 解耦。

4. 使用 Buffer Pool、BlockingQueue 等工程优化降低延迟抖动，提高长期运行稳定性。

5. 在 RK3588 平台实现：

   * 1.5ms NPU推理
   * 8ms端到端延迟
   * 100+ FPS实时处理能力

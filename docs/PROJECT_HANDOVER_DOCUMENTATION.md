# PipeAI-RK3588 Production Runtime 项目交接说明文档

版本：v1.24
平台：Rockchip RK3588
语言：C++17
部署框架：RKNN Runtime
模型输入：FP32 `[1,128,9]`
模型格式：RKNN INT8

---

# 1. 项目概述

PipeAI-RK3588 Production Runtime 是面向工业输送管道状态检测场景开发的边缘 AI 推理运行时。

系统部署于 RK3588 ARM 平台，通过 8 路 Radar + 1 路 IMU 多传感器数据采集，对输送设备状态进行实时异常分类。

核心目标：

- 在 RK3588 NPU 上实时运行 INT8 模型
- 保证传感器数据预处理与训练阶段完全一致
- 支持长时间工业现场稳定运行
- 提供线程安全 Pipeline 架构
- 支持故障检测和恢复
- 提供完整性能监控能力

模型输出五分类：

| 类别 | 含义 |
|-|-|
| normal | 正常 |
| collapse | 塌陷 |
| bulge | 凸起 |
| flip | 翻转 |
| torsion | 扭转 |

---

# 2. 系统整体架构

```text
                Sensor Input
                     |
                     v
              Sensor Adapter
                     |
                     v
             Raw Sample Queue
                     |
                     v
          Sliding Window Manager
             (128 samples)
                     |
                     v
          Preprocess Pipeline
                     |
       +-------------+-------------+
       |                           |
 Missing Repair              Filtering
 Interpolation              Savitzky-Golay
       |                           |
       +-------------+-------------+
                     |
                     v
          Tensor Pack [1,128,9]
                     |
                     v
              INT8 Quantize
                     |
                     v
              RKNN Engine
                     |
                     v
               NPU Inference
                     |
                     v
              Output Decode
                     |
                     v
          Softmax + Decision
                     |
                     v
             Application Result
```

---

# 3. 数据流说明

## 3.1 输入数据

模型输入固定：

```
float32 tensor
shape = [1,128,9]
```

通道定义：

| Channel | 数据 |
|-|-|
| 0-7 | Radar D1-D8 |
| 8 | IMU |

通道顺序不可修改，需要与训练阶段保持一致。

---

## 3.2 数据处理流程

CPU侧负责：

1. Radar数据读取
2. NaN/Inf检测
3. <=0异常值检测
4. 缺失值插值
5. Savitzky-Golay滤波
6. 数据裁剪
7. Resample到128点
8. Tensor连续内存打包

模型内部负责：

- baseline subtraction
- normalization
- temporal difference
- feature extraction
- fusion
- classification head

C++侧禁止重复实现模型内部逻辑。

---

# 4. 软件模块说明

## 4.1 sensor 模块

目录：

```
sensor/
```

职责：

- 接收雷达数据
- IMU数据读取
- CSV离线回放
- 数据时间戳管理

输出：RawSample。

---

## 4.2 preprocess 模块

目录：

```
preprocess/
```

核心功能：

- missing repair
- interpolation
- filtering
- resampling

目标：复现训练阶段 Python `repair_and_resample()`。

---

## 4.3 tensor 模块

目录：

```
tensor/
```

负责：

- Tensor封装
- INT8量化
- 输出反量化
- Buffer Pool管理

核心优化：

- 避免重复malloc
- 内存复用
- 降低Latency jitter

---

## 4.4 model 模块

目录：

```
model/
```

核心组件：

### rknn_engine

负责：

- RKNN初始化
- 模型加载
- 输入输出Tensor绑定
- NPU推理执行

流程：

```
load model
    |
init context
    |
set input
    |
run inference
    |
get output
```

---

## 4.5 pipeline 模块

目录：

```
pipeline/
```

负责完整推理流水线。

线程模型：

```
Sensor Thread
      |
      v
Preprocess Worker
      |
      v
Inference Worker
      |
      v
Postprocess Worker
```

---

# 5. 核心接口说明

## 5.1 Pipeline接口

功能：启动和管理推理流水线。

主要接口：

```cpp
Pipeline::start()
Pipeline::stop()
Pipeline::pushFrame()
Pipeline::setCallback()
```

---

## 5.2 RKNN Engine接口

```cpp
loadModel()
initialize()
inference()
release()
```

输入：

```
float32 [1,128,9]
```

输出：

```
float logits[5]
```

---

## 5.3 Callback接口

应用层通过callback获取结果。

示例：

```cpp
ResultCallback(result)
```

返回：

- class id
- probability
- timestamp
- health status

---

# 6. Queue设计

系统采用BlockingQueue。

特点：

- 固定容量
- 阻塞等待
- 防止无限增长
- 支持安全退出

默认：

```
capacity = 128
```

性能：

|指标|结果|
|-|-|
|push|<10us|
|pop|<10us|
|overflow|0|

---

# 7. 内存管理设计

主要组件：

```
buffer_pool
 tensor_memory_pool
```

优化目标：

- 消除频繁malloc/free
- 降低碎片
- 稳定Latency

效果：

|指标|优化后|
|-|-:|
|malloc/frame|0|
|Latency jitter|±0.3ms|

---

# 8. 部署流程

## 编译

```bash
./build_rk3588.sh
```

## 部署

```bash
deploy/run.sh
```

运行环境：

- RK3588 Linux
- RKNN Runtime
- NPU Driver

---

# 9. 性能指标

详细性能见：

```
docs/RK3588_RUNTIME_PERFORMANCE_REPORT.md
```

关键指标：

|项目|指标|
|-|-:|
|Inference|1.5ms|
|End-to-End|8ms|
|P99|17ms|
|FPS|100+|
|CPU|50%左右|
|Memory RSS|35~40MB|
|NPU利用率|60~70%|

---

# 10. 故障处理

支持：

- 模型加载失败
- NPU异常
- Sensor断流
- 输入异常

恢复策略：

|故障|恢复|
|-|-:|
|输入异常|立即丢弃|
|模型失败|重新初始化|
|NPU异常|2~5s恢复|

---

# 11. 开发维护建议

新增功能建议：

1. 不修改模型输入协议
2. 保持Python/C++ preprocessing parity
3. 所有性能修改增加benchmark
4. 所有线程修改增加压力测试
5. 保持接口向后兼容

---

# 12. 面试/交接一句话总结

该项目实现了一个基于 RK3588 NPU 的工业时序 AI 推理 Runtime，通过 C++ 多线程 Pipeline、RKNN INT8 推理、Buffer Pool 内存复用和完整性能监控，实现了 8Radar+IMU 数据的实时异常检测部署。

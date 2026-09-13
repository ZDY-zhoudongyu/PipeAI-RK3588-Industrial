# PipeAI RK3588 Production Runtime

# API Reference & Call Sequence Guide

版本：v1.24

本文档用于代码级交接，描述核心类、接口职责、调用关系以及扩展方式。

---

# 1. 软件分层结构

```
Application
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
+----------------+
| Postprocess    |
+----------------+
        |
Result Callback
```

---

# 2. 核心模块 API

## 2.1 Application

文件：

```
runtime/application.hpp
```

职责：

- 系统启动入口
- 初始化Runtime
- 管理生命周期

主要接口：

```cpp
bool initialize();

bool start();

void stop();
```

调用关系：

```
main()
 |
Application::initialize()
 |
Pipeline::initialize()
 |
RKNN Engine Load
```

---

# 3. Pipeline API

文件：

```
pipeline/pipeline.hpp
```

Pipeline负责完整推理流水线。

## 核心接口

```cpp
bool start();

void stop();

bool pushFrame(FrameContext frame);

void setResultCallback(callback);
```

数据流：

```
Sensor
 |
FrameContext
 |
BlockingQueue
 |
Worker Thread
 |
Inference
 |
Callback
```

---

# 4. FrameContext

文件：

```
pipeline/frame_context.hpp
```

用于线程间传递单帧数据。

包含：

```cpp
struct FrameContext
{
    timestamp;
    sensor_data;
    tensor;
    result;
};
```

生命周期：

```
Create
 |
Preprocess
 |
Inference
 |
Release
```

---

# 5. Sensor Interface

文件：

```
sensor/sensor_interface.hpp
```

抽象传感器接口。

接口：

```cpp
virtual bool open();

virtual bool read();

virtual void close();
```

支持：

- CSV Replay
- 实际Radar设备
- 自定义Sensor Adapter

---

# 6. Preprocess API

目录：

```
preprocess/
```

处理流程：

```
Raw Radar
 |
Missing Repair
 |
Savitzky Golay Filter
 |
Resample
 |
Normalize
 |
Tensor Pack
```

核心接口：

```cpp
bool process(
    FrameContext& frame
);
```

主要组件：

|模块|作用|
|-|-|
|missing_repair|异常值修复|
|savgol|平滑滤波|
|resample|窗口重采样|
|normalize|归一化|

---

# 7. Tensor Layer API

目录：

```
tensor/
```

职责：

- Tensor生命周期管理
- INT8量化
- Buffer复用

核心接口：

```cpp
Tensor allocate();

bool quantize(float*, int8_t*);

void release();
```

优化：

- buffer pool
- memory pool
- zero allocation runtime

---

# 8. RKNN Engine API

文件：

```
model/rknn_engine.hpp
```

负责NPU推理。

核心接口：

```cpp
bool loadModel(path);

bool initialize();

bool inference(
    Tensor& input,
    Tensor& output
);

void release();
```

调用流程：

```
load model
    |
create RKNN context
    |
query tensor info
    |
set input
    |
run inference
    |
get output
```

---

# 9. Postprocess API

目录：

```
postprocess/
```

流程：

```
INT8 output
 |
unpack
 |
softmax
 |
classification
 |
result callback
```

接口：

```cpp
Result classify(output);
```

输出：

```cpp
class_id
confidence
timestamp
```

---

# 10. Callback接口

文件：

```
pipeline/result_callback.hpp
```

用户应用通过callback获取结果。

示例：

```cpp
void onResult(Result result)
{
    // alarm
    // storage
    // display
}
```

---

# 11. 多线程调用时序

```
Sensor Thread
      |
      v
pushFrame()
      |
      v
Preprocess Worker
      |
      v
Tensor Queue
      |
      v
Inference Worker
      |
      v
RKNN NPU
      |
      v
Postprocess Worker
      |
      v
Callback
```

---

# 12. 异常处理流程

```
Error
 |
RecoveryManager
 |
+-------------+
| Retry       |
+-------------+
 |
Failed
 |
Restart Model
 |
Watchdog
```

---

# 13. 扩展指南

## 新增Sensor

实现：

```
SensorInterface
```

无需修改Pipeline。

---

## 替换模型

步骤：

1. 导出ONNX
2. RKNN转换
3. 更新model文件
4. 修改tensor shape
5. 更新postprocess类别

---

## 增加新的Preprocess

新增模块：

```
preprocess/new_xxx.cpp
```

注册pipeline即可。

---

# 14. 调试工具

推荐：

|工具|用途|
|-|-|
|RKNN profiler|NPU耗时|
|perf|CPU分析|
|top|资源监控|
|trace|线程调度|
|benchmark|Latency统计|

---

# 15. 维护原则

- Pipeline接口保持稳定
- Sensor通过接口隔离
- Model通过RKNN Engine隔离
- Tensor统一Buffer管理
- 禁止业务代码直接操作NPU


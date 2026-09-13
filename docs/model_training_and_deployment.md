# 管带机 8 路雷达 + IMU 端到端双流模型（RK3588 版）

## 1. 设计目标

输入不再是提前生成的 PNG + NPY，而是一个固定长度的原始传感器张量：

- 输入 shape：`[1, 128, 9]`
- 0~7 列：D1~D8 雷达
- 第 8 列：IMU
- 输出 shape：`[1, 5]`
- 类别：normal / collapse / bulge / flip / torsion

模型内部自动完成：

1. 正常基线扣除；
2. 空间布局图构建；
3. 原始雷达、一阶差分、二阶差分、IMU 时序特征生成；
4. 2D 空间分支与 1D 时序分支提取；
5. 双流融合与五分类。

CSV 的缺失值修复、Savitzky-Golay 轻平滑、长度重采样放在模型外 CPU 端执行。这样做是为了保证 ONNX/RKNN 图静态、简单、易量化，同时不会把文件 I/O、插值和异常值修复硬塞进 NPU 图。

## 2. “带空间布局”的图像化方法

旧方法把 8 路雷达直接堆成 `8×128`，纵坐标实际上只是 D1~D8 的编号，并不能表达 D1-D4 密集、D5-D8 稀疏的真实拱形安装几何关系。

新方法把 8 个雷达按照安装角度放到上半圆截面的真实二维位置，每个雷达生成一个高斯空间影响场，然后把每路雷达在一个窗口内的：

- 平均形变量；
- 最大正向形变；
- 最大负向形变；

投影成 3 通道 `64×64` 半圆截面空间图。这样图像像素位置与实际雷达物理位置存在直接对应关系。

**特别重要：** 当前代码里的 `sensor_angles_deg=(15,30,45,60,90,120,145,165)` 只是根据报告中“D1-D4 密集、D5-D8 稀疏”的描述给出的可运行示例，不是实际安装角度。最终论文实验和现场部署必须替换成你支架上 D1~D8 的实测角度，否则不能宣称“严格保留真实空间布局”。

## 3. 模型量级

当前轻量版本（固定输入 128×9）：

- 可训练参数：50,605
- 权重 FP32 理论体积：约 0.193 MiB
- 权重 FP16 理论体积：约 0.097 MiB
- 权重 INT8 理论体积：约 0.048 MiB
- MAC：约 1.297 M / 样本
- FLOPs：约 2.595 M / 样本（按 1 MAC=2 FLOPs）

原始双流网络按提供代码估算：

- 参数：992,993
- MAC：约 170.514 M
- FLOPs：约 341.028 M

所以轻量版参数量约为原模型的 5.1%，MAC 约为原模型的 0.76%。

> RKNN 文件还包含图结构、量化参数和常量，因此最终 `.rknn` 文件大小会高于纯权重理论体积。

## 4. 训练

```bash
python pipe_rk3588_end2end.py \
  --data-dir D:/graduate/generated_dataset_v56 \
  --out-dir D:/graduate/rk3588_end2end
```

输出：

- `best_end2end.pth`
- `train_stats.npz`
- `history.csv`
- `config.json`

只查看理论参数/FLOPs：

```bash
python pipe_rk3588_end2end.py --profile
```

## 5. 导出 ONNX

安装：

```bash
pip install onnx
```

然后：

```bash
python export_onnx.py \
  --work-dir D:/graduate/rk3588_end2end \
  --output pipe_end2end.onnx
```

导出是固定 batch=1、固定长度 128，不使用动态 shape。

## 6. 生成 INT8 校准数据

```bash
python make_calibration_dataset.py \
  --data-dir D:/graduate/generated_dataset_v56 \
  --out-dir D:/graduate/rk3588_calib \
  --num 300
```

应尽量保证 5 类样本均衡，并覆盖实际现场可能出现的温漂、振动、轻微偏移、早期微弱形变。

## 7. ONNX -> RKNN

在安装 RKNN-Toolkit2 的 Ubuntu/x86 主机执行：

```bash
python convert_rknn.py \
  --onnx D:/graduate/rk3588_end2end/pipe_end2end.onnx \
  --dataset D:/graduate/rk3588_calib/dataset.txt \
  --output pipe_end2end_int8.rknn
```

如果 INT8 精度下降明显，先导出不量化 FP16 版做基准：

```bash
python convert_rknn.py \
  --onnx pipe_end2end.onnx \
  --dataset dataset.txt \
  --output pipe_end2end_fp16.rknn \
  --no-quant
```

## 8. RK3588 板端测试

板端安装与板卡系统匹配的 RKNNLite2 后：

```bash
python infer_rk3588.py \
  --model pipe_end2end_int8.rknn \
  --csv one_sample.csv \
  --loops 100
```

脚本会输出分类概率以及 mean / p50 / p95 推理延迟。

## 9. 真实部署必须注意

1. **传感器物理编号绝对不能改变。** D1~D8 的顺序就是空间位置，换线或软件枚举顺序变化会直接破坏模型空间语义。
2. **角度需要标定。** 支架调整后要同步更新 `sensor_angles_deg`，重新训练或至少重新验证。
3. **基线不能永久固定。** 现场温度、托辊磨损、支架变化会造成长期漂移。建议维护“确认正常工况”的慢更新基线，但发生告警时冻结更新。
4. **不要用单帧分类直接报警。** 建议 128 点滑窗、步长 16~32 点，连续 3~5 个窗口同类高置信度才触发故障。
5. **增加未知/不可信逻辑。** 五分类 softmax 最大概率过低、输入越界、某路雷达长时间常值、丢帧率过高时，应输出 `sensor_fault/unknown`，而不是强行归入某个故障类别。
6. **相机只作为告警后的复核。** 传感模型触发后保存前后 15 s 视频与原始 9 通道时序，便于追溯。
7. **训练/测试必须按原始长时序或采集批次隔离。** 不能先切重叠窗口再随机拆 train/val/test，否则相邻高度相似窗口会泄漏。
8. **当前 v56 数据增强中的“随机通道反转”不适合真实空间拓扑模型。** 雷达距离信号取负在物理上没有意义，而且会改变故障方向。建议删除；大幅独立通道缩放（0.6~1.6）也应降到更接近传感器标定误差的范围。
9. **不要把 `SENSOR_MIN=150` 当成唯一异常检测。** 还需要检查上限、跳变速度、连续常值、CRC/串口超时、时间戳错位。
10. **量化验证要单独做。** 至少对比 PyTorch FP32、ONNX、RKNN FP16、RKNN INT8 四级输出，记录整体准确率和每类召回率变化。
11. **关注弱故障召回而不是只看总准确率。** 特别是 bulge/flip/torsion 的早期微弱样本，应重点查看 Recall、F1、混淆矩阵和连续告警延迟。
12. **性能数字必须在目标工业板实测。** RK3588 的理论 NPU 算力不能直接换算成本模型真实延迟，实际速度受 RKNN 编译、DDR、输入拷贝、CPU 预处理、NPU 核调度和系统温控影响。

## 10. 推荐的现场在线逻辑

```text
8路雷达 + IMU 同步采集
        ↓
时间戳对齐 / 无效值检查
        ↓
128点环形缓冲区
        ↓  每16~32点触发一次
缺失修复 + 轻平滑 + 重采样（必要时）
        ↓
RKNN 单输入 [1,128,9]
        ↓
5类概率 + 输入健康度判断
        ↓
时间去抖（连续3~5窗）
        ↓
正常 / 塌管 / 胀管 / 反包 / 扭转 / Unknown
        ↓
告警、保存CSV、保存前后15 s视频、上传上位机
```

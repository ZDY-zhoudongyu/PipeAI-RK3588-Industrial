from rknn.api import RKNN

rknn = RKNN()
rknn.config(target_platform='rk3588')
rknn.load_onnx(model='../onnx/pipe_model.onnx')
rknn.build(do_quantization=True, dataset='./dataset.txt')
rknn.export_rknn('../rknn/pipe_model.rknn')
print('built')

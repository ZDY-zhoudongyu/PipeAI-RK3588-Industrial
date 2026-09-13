import torch
from model import PipeNet

model = PipeNet()
model.eval()

x = torch.randn(1,128,9)
torch.onnx.export(model, x, '../onnx/pipe_model.onnx', input_names=['input'], output_names=['output'], opset_version=12)
print('exported')

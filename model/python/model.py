"""PipeAI model definition placeholder.
Replace this network with the trained PyTorch dual-stream model.
Input: [B,128,9], output: 5 classes.
"""
import torch
import torch.nn as nn

class PipeNet(nn.Module):
    def __init__(self, classes=5):
        super().__init__()
        self.net = nn.Sequential(
            nn.Conv1d(9, 32, 3, padding=1),
            nn.ReLU(),
            nn.AdaptiveAvgPool1d(1),
            nn.Flatten(),
            nn.Linear(32, classes)
        )
    def forward(self, x):
        x = x.transpose(1,2)
        return self.net(x)

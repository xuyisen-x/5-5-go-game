import torch
import torch.nn as nn
import torch.nn.functional as F

class BN_Conv2d(nn.Module):
    """
    BN_CONV, default activation is nn.ReLU
    set activation=None to disable activation
    """
    def __init__(self, in_planes: int, out_planes: int, kernel_size: object, stride: object, padding: object,
                 dilation=1, groups=1, bias=False, activation=nn.ReLU) -> object:
        super(BN_Conv2d, self).__init__()
        self.layers = nn.Sequential(
            nn.Conv2d(in_planes, out_planes, kernel_size=kernel_size, stride=stride,
                            padding=padding, dilation=dilation, groups=groups, bias=bias),
            nn.BatchNorm2d(out_planes)
        )
        if (activation != None):
            self.layers.append(activation())
    def forward(self, x):
        return self.layers(x)
    
class BasicBlock(nn.Module):
    def __init__(self, in_planes: int, out_planes:int, kernel_size:int, padding: object, stride: object) -> object:
        super(BasicBlock, self).__init__()
        self.layers = nn.Sequential(
            BN_Conv2d(in_planes, out_planes, kernel_size=kernel_size, stride=stride, padding=padding),
            BN_Conv2d(out_planes, out_planes, kernel_size=kernel_size, stride=1, padding=padding, activation=None)
        )
        self.shortcut = nn.Identity()
        if (stride != 1 or in_planes != out_planes):
            self.shortcut = nn.Sequential(
                nn.Conv2d(in_planes, out_planes, kernel_size=1, stride=1, bias=False),
                nn.MaxPool2d(kernel_size=kernel_size, stride=stride, padding=padding),
                nn.BatchNorm2d(out_planes)
            )
    def forward(self, x):
        out = self.layers(x)
        out += self.shortcut(x)
        out = F.relu(out)
        return out
    
class policyNet(nn.Module):
    def __init__(self):
        super(policyNet, self).__init__()
        self.conv0 = nn.Sequential(nn.Conv2d(5, 64, kernel_size=3, padding=1),nn.ReLU())
        self.conv1x = BasicBlock(64, 64, stride=1, kernel_size=3, padding=1)
        self.conv2x = BasicBlock(64, 64, stride=1, kernel_size=3, padding=1)
        self.conv3x = BasicBlock(64, 1, stride=1, kernel_size=3, padding=1)
        self.output = nn.Sequential(nn.Flatten(), nn.Linear(25, 128), nn.ReLU(), nn.Linear(128, 26))
    def forward(self, x):
        x = self.conv0(x)
        x = self.conv1x(x)
        x = self.conv2x(x)
        x = self.conv3x(x)
        x = self.output(x)
        return x
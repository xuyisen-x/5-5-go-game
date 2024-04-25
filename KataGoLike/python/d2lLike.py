import time

from matplotlib import pyplot as plt
from IPython.display import clear_output as clear, display
import torch
from torch import nn
import numpy as np

class Animator:
    def __init__(self, xlabel = None, ylabel = None, title = None, xlim = None, ylim = None, legend = None):
        self.fig = plt.figure()
        self.ax = self.fig.add_subplot(111)
        self.ax.grid()
        if xlabel: self.ax.set_xlabel(xlabel)
        if ylabel: self.ax.set_ylabel(ylabel)
        if title: self.ax.set_title(title)
        if xlim: self.ax.set_xlim(xlim)
        if ylim: self.ax.set_ylim(ylim)
        self.legend = legend
        self.numOfLines = len(legend) if legend else 1
        self.x = [[] for _ in range(self.numOfLines)]
        self.y = [[] for _ in range(self.numOfLines)]
        self.lines = [self.ax.plot([], [])[0] for _ in range(self.numOfLines)]
        
    def add(self, x, ys):
        assert(len(ys) == self.numOfLines)
        for i, y in enumerate(ys):
            if y is not None:
                self.x[i].append(x)
                self.y[i].append(y)
                self.lines[i].set_data(self.x[i], self.y[i])
        if self.legend: 
            self.ax.legend(self.legend)
        self.ax.relim()
        self.ax.autoscale_view()
        display(self.fig)
        clear(wait=True)
        
def evaluate_accuracy(net, data_iter, device=None):
    if isinstance(net, nn.Module):
        net.eval()  # 设置为评估模式
        if not device:
            device = next(iter(net.parameters())).device
    # 正确预测的数量，总预测的数量
    metric = Accumulator(2)
    with torch.no_grad():
        for X, y in data_iter:
            if isinstance(X, list):
                # BERT微调所需的（之后将介绍）
                X = [x.to(device) for x in X]
            else:
                X = X.to(device)
            y = y.to(device)
            metric.add(accuracy(net(X), y), y.numel())
    return metric[0] / metric[1]

class Accumulator:
    """For accumulating sums over `n` variables."""
    def __init__(self, n):
        """Defined in :numref:`sec_utils`"""
        self.data = [0.0] * n

    def add(self, *args):
        self.data = [a + float(b) for a, b in zip(self.data, args)]

    def reset(self):
        self.data = [0.0] * len(self.data)

    def __getitem__(self, idx):
        return self.data[idx]
    
def accuracy(y_hat, y):
    """Compute the number of correct predictions.

    Defined in :numref:`sec_utils`"""
    y_hat = torch.sigmoid(y_hat)
    y_hat = (y_hat > 0.5)
    cmp = y_hat.type_as(y) == y
    return float(cmp.sum())

class Timer:
    """Record multiple running times."""
    def __init__(self):
        """Defined in :numref:`sec_minibatch_sgd`"""
        self.times = []
        self.start()

    def start(self):
        """Start the timer."""
        self.tik = time.time()

    def stop(self):
        """Stop the timer and record the time in a list."""
        self.times.append(time.time() - self.tik)
        return self.times[-1]

    def avg(self):
        """Return the average time."""
        return sum(self.times) / len(self.times)

    def sum(self):
        """Return the sum of time."""
        return sum(self.times)

    def cumsum(self):
        """Return the accumulated time."""
        return np.array(self.times).cumsum().tolist()
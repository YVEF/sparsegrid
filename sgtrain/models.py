import torch
import torch.nn as nn


class SgModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.L1 = torch.nn.Linear(320, 545, dtype=torch.float64)
        self.l1_act = nn.LeakyReLU(negative_slope=0.1)
        # self.l1_act = nn.ReLU6()
        # self.l1_act = nn.Sigmoid()
        self.L2 = torch.nn.Linear(545, 170, dtype=torch.float64)
        self.l2_act = nn.ReLU6()
        # self.l2_act = nn.Sigmoid()
        self.Lout = torch.nn.Linear(170, 1, dtype=torch.float64)
        self.out_act = nn.Sigmoid()

    def forward(self, x):
        h1 = self.L1(x)
        h1_a = self.l1_act(h1)
        h2 = self.L2(h1_a)
        h2_a = self.l2_act(h2)
        logits = self.Lout(h2_a)
        out = self.out_act(logits)
        return out

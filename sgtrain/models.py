import torch
import torch.nn as nn
import torch.jit as jit


# class SgModel(nn.Module):
class SgModel(jit.ScriptModule):
    def __init__(self):
        super().__init__()

        self.BATCH_SIZE = 40
        self.INPUT_SIZE = 320
        self.OUTPUT_SIZE = 128
        self.KERNEL_SIZE = int(self.INPUT_SIZE / self.BATCH_SIZE)

        self.l1 = (nn.Conv1d(self.BATCH_SIZE, self.OUTPUT_SIZE,
                             kernel_size=self.KERNEL_SIZE,
                             bias=False, dtype=torch.float64))

        self.l1_act = nn.LeakyReLU(negative_slope=0.1)
        self.l2 = nn.Linear(128, 64, dtype=torch.float64)
        self.l2_act = nn.LeakyReLU(negative_slope=0.3)
        self.lout = nn.Linear(64, 1, dtype=torch.float64)
        self.out_act = nn.Sigmoid()

    @jit.script_method
    def forward(self, x):
        inp = x.view(self.BATCH_SIZE, -1)
        h1 = self.l1_act(self.l1(inp)).view(1, -1).squeeze(0)
        h2 = self.l2_act(self.l2(h1))
        return self.out_act(self.lout(h2))


import torch
import torch.nn as nn
import torch.jit as jit


# class SgModel(nn.Module):
class SgModel(jit.ScriptModule):
    # SG_MODEL_WEIGHTS_FILE = "sgw_22.pt"
    SG_MODEL_WEIGHTS_FILE = "sgw.pt"

    def __init__(self):
        super().__init__()
        self.l1 = nn.Linear(320, 545, dtype=torch.float64).double()
        self.l1_act = nn.LeakyReLU(negative_slope=0.1)
        self.l2 = nn.Linear(545, 170, dtype=torch.float64).double()
        # self.l2_act = nn.ReLU6()
        self.l2_act = nn.LeakyReLU(negative_slope=0.3)
        self.lout = nn.Linear(170, 1, dtype=torch.float64).double()
        self.out_act = nn.Sigmoid()

    @jit.script_method
    def forward(self, x):
        h1 = self.l1_act(self.l1(x))
        h2 = self.l2_act(self.l2(h1))
        return self.out_act(self.lout(h2))

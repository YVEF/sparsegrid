from ctypes import cdll
import sg_trainer_interop as sgt
import torch
import os


class SgModel(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.L1 = torch.nn.Linear(320, 820)
        self.L2 = torch.nn.Linear(820, 150)
        self.Lout = torch.nn.Linear(150, 1)

    def forward(self, x):
        h1 = self.L1(x)
        h1_a = torch.tanh(h1)
        h2 = self.L2(h1_a)
        h2_a = torch.tanh(h2)
        out = self.Lout(h2_a)
        out_a = torch.tanh(out)
        return out_a


def train(model, gamesCount, inputLayer, lr_ = 0.1):
    optimizer = torch.optim.SGD(model.parameters(), lr=lr_, momentum=0.5)
    for game in range(gamesCount):
        print(game)

    return



def run():
    cdc = sgt.initCDC(True)
    sgt.welcome(cdc)

    # for i in range(moveCollection.size):
    #     mv = moveCollection.getMove(i)
    #     print(mv.fromSq, mv.toSq)

    inputLayer = sgt.initNNInputLayer()
    sgt.fillInputLayer(cdc, inputLayer)
    print(inputLayer)

    model = SgModel()
    params_file = "weights.csv"
    # if os.path.isfile(params_file):
    #     restore_model_parameters(model, params_file)

    train(model, 100, inputLayer)
    # moveCollection = sgt.nextMoves(cdc, True)
    # print(moveCollection.size)

    return


if __name__ == "__main__":
    run()

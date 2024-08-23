from ctypes import cdll
import sg_trainer_interop as sgt
import torch
import torch.nn as nn
import torch.optim as optim
import os
import random
import numpy as np
import chess
import mct
import gamesdb as gd

random.seed(123)

class SgModel(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.L1 = torch.nn.Linear(320, 820)
        self.l1_act = nn.Tanh()
        self.L2 = torch.nn.Linear(820, 150)
        self.l2_act = nn.Tanh()
        self.Lout = torch.nn.Linear(150, 1)
        self.outNonLin = nn.Softmax(dim=1)

    def forward(self, x):
        h1 = self.L1(x)
        h1_a = self.l1_act(h1)
        h2 = self.L2(h1_a)
        h2_a = self.l2_act(h2)
        logits = self.Lout(h2_a)
        out = self.outNonLin(logits)
        return out

def rollout(node: mct.MCTNode, cdc):
    assert len(node.children) == 0

    moves = sgt.nextMoves(cdc, node.white)
    print(moves)

    res = 0
    if moves.size != 0:
        i = np.random.randint(moves.size)
        move = moves.getMove(i)
        sgt.makeMove(cdc, move)
        res = rollout(node, cdc)
    else:
        res = 1.0
        # calc action-value
        pass

    sgt.undoMove(cdc)
    return res





def MCTS(mct):
    pass



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
    gd.run_human_test()
    # run()

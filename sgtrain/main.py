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
import gamesdb
import selfplay


# Here is 2 types of training on the moment:
# 1) Games db: huge games db train the model predict the full precise trajectory
#       - Backpropagation and optimization of the fly without actuall game tree
# 2) Game simulation: play with various agent (e.g. self-playing, Stockfish etc...)
#       - Here the MCTS is actully in use


if __name__ == "__main__":
    # gamesdb.start(4, 4)
    selfplay.start(4, 4)

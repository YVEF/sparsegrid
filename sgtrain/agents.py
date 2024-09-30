import copy
import time
import numpy
import numpy as np
import torch
import torch.nn as nn
import sg_trainer_interop as sgt
import math
import os
import models
from abc import ABC
import sgfiles


class Agent(ABC):
    def __init__(self, color, create_optim, optim_state_file, mod = None):
        self._color = color
        self._model = models.SgModel() if mod is None else mod
        self._cdc = sgt.initCDC()
        self._in_layer = sgt.initNNLayer()
        self._actions = []
        self._moves_number = 0
        self._optimizer = create_optim(self._model)
        self._optim_state_file = optim_state_file

    def get_cdc(self):
        return self._cdc

    def _discound_reward(self, reward, step_count, gamma=0.99):
        assert step_count > 0
        rewards = np.zeros(step_count, dtype=np.float64)
        rewards[-1] = reward * gamma
        for i in reversed(range(0, step_count - 1)):
            rewards[i] = rewards[i + 1] * gamma

        return rewards

    def reset(self):
        sgt.freeCDC(self._cdc)
        self._cdc = sgt.initCDC()
        self._in_layer = sgt.initNNLayer()
        self._actions = []
        self._moves_number = 0

    def restore_state(self):
        if os.path.isfile(sgfiles.SG_MODEL_WEIGHTS_FILE):
            self._model.load_state_dict(torch.load(sgfiles.SG_MODEL_WEIGHTS_FILE, weights_only=True))
        if os.path.isfile(self._optim_state_file):
            self._optimizer.load_state_dict(torch.load(self._optim_state_file, weights_only=True))

    def evaluate(self):
        assert self._moves_number > 0
        pred = self._model(torch.tensor(self._in_layer, dtype=torch.float64))
        self._actions.append(pred)

    def color(self):
        return self._color

    def set_side(self, color):
        self._color = color

    def make_move(self, mv):
        sgt.makeMove(self._cdc, mv, self._in_layer)
        self._moves_number += 1

    def step(self, reward):
        assert len(self._actions) > 0
        rewards = self._discound_reward(reward, len(self._actions))
        epsilon = 1e-10
        loss_error = 0.
        loss = torch.tensor([0.], requires_grad=True).double()
        for r, p in zip(rewards, self._actions):
            loss_error += abs(max(reward, 0.0) - p.item())
            loss += (r * torch.log(torch.clamp(p, min=epsilon, max=1-epsilon)))

        loss = -loss.mean()
        self._optimizer.zero_grad()
        loss.backward()
        self._optimizer.step()
        return loss_error

    def checkpoint(self):
        torch.save(self._model.state_dict(), sgfiles.SG_MODEL_WEIGHTS_FILE)
        torch.save(self._optimizer.state_dict(), self._optim_state_file)

    def switch_side(self):
        self._color = not self._color
        self.reset()

    def display_board(self):
        sgt.displayBoard(self._cdc)

    def get_last_evaluation(self):
        assert len(self._actions) > 0
        return self._actions[-1].item()

    def make_move_uci(self, from_sq, to_sq):
        mv = sgt.recognizeMove(self._cdc, from_sq, to_sq)
        self.make_move(mv)


class GamesDbAgent(Agent):
    def __init__(self, color, lr_=1e-3):
        super().__init__(
            color,
            lambda m : torch.optim.SGD(m.parameters(), lr=lr_, weight_decay=2e-3),
            sgfiles.SGD_OPTIM_STATE_FILE)

    def get_model(self):
        return self._model


class SelfPlayAgent(Agent):
    def __init__(self, color, lr_=1e-4):
        super().__init__(
            color,
            # lambda m : torch.optim.Adam(m.parameters(), lr=lr_),
            lambda m : torch.optim.SGD(m.parameters(), lr=lr_, weight_decay=3e-2),
            sgfiles.ADAM_OPTIM_STATE_FILE)

    def get_next_move(self):
        moves = sgt.nextMoves(self._cdc, self._color)
        m = 0.
        move = None
        for i in range(0, moves.size):
            mv = moves.getMove(i)
            sgt.makeMoveSilently(self._cdc, mv)
            pred = self._model(torch.from_numpy(self._in_layer)).item()
            sgt.undoMoveSilently(self._cdc)
            if m < pred or move is None:
                m = pred
                move = mv
        return move

    def is_draw(self):
        return sgt.isDraw(self._cdc)


rank = ['1','2','3','4','5','6','7','8']
file = ['a','b','c','d','e','f','g','h']


def move_to_uci_str(move):
    return (f''
            f'{file[int(move.fromSq%8)]}'
            f'{rank[int(move.fromSq/8)]}'
            f'{file[int(move.toSq%8)]}'
            f'{rank[int(move.toSq/8)]}')


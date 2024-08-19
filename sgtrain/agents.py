import copy
import numpy
import numpy as np
import torch
import torch.nn as nn
import sg_trainer_interop as sgt
import math
import os
import models
from abc import ABC


# torch.manual_seed(123)

class Agent(ABC):
    def __init__(self, color):
        self._color = color
        self._model = models.SgModel()
        self._cdc = sgt.initCDC()
        self._in_layer = sgt.initNNLayer()
        sgt.rebuildNN(self._cdc, self._in_layer)
        self._actions = []

    def _accumulate_reward(self, reward, step_count, gamma=0.99):
        assert step_count > 0
        rewards = np.zeros(step_count, dtype=np.float64)
        rewards[-1] = reward
        # y = 0
        for i in reversed(range(0, step_count - 1)):
            rewards[i] = rewards[i + 1] * gamma
            # y = y * gamma + rewards[i]
            # result[i] = y

        # skip normalization
        # rewards = (rewards - rewards.mean()) / (rewards.std() + 1e-8)
        return rewards

    def reset(self):
        sgt.freeCDC(self._cdc)
        self._cdc = sgt.initCDC()
        self._in_layer = sgt.initNNLayer()
        sgt.rebuildNN(self._cdc, self._in_layer)
        self._actions = []

    def _checkpoint_base(self,):
        torch.save(self._model.state_dict(), self._model.SG_MODEL_WEIGHTS_FILE)

    def _restore_state_base(self):
        if os.path.isfile(self._model.SG_MODEL_WEIGHTS_FILE):
            self._model.load_state_dict(torch.load(self._model.SG_MODEL_WEIGHTS_FILE, weights_only=True))
            return True
        return False

    def color(self):
        return self._color

    def set_side(self, color):
        self._color = color

    def make_move(self, mv):
        sgt.makeMove(self._cdc, mv)
        sgt.fillNNLayer(self._cdc, self._in_layer)
        # sgt.rebuildNN(self._cdc, self._in_layer)


class GamesDbAgent(Agent):
    _OPTIMIZER_STATE_FILE = "sgs.pt"

    def __init__(self, color, lr_=1e-4):
        super().__init__(color)
        self._optimizer = torch.optim.SGD(self._model.parameters(),
                                          lr=lr_, weight_decay=0.03, momentum=0.5, nesterov=True)

    def checkpoint(self):
        self._checkpoint_base()
        torch.save(self._optimizer.state_dict(), self._OPTIMIZER_STATE_FILE)

    def restore_state(self):
        if self._restore_state_base():
            self._optimizer.load_state_dict(torch.load(self._OPTIMIZER_STATE_FILE))

    def make_move_uci(self, from_sq, to_sq):
        mv = sgt.recognizeMove(self._cdc, from_sq, to_sq)
        self.make_move(mv)

    def switch_side(self):
        self._color = not self._color
        self.reset()

    def evaluate(self):
        pred = self._model(torch.from_numpy(self._in_layer))
        self._actions.append(pred)

    def step(self, reward):
        rewards = self._accumulate_reward(reward, len(self._actions))

        epsilon = 1e-11
        loss_error = 0.
        loss = torch.tensor([0.], requires_grad=True).double()

        for r, p in zip(rewards, self._actions):
            loss_error += abs(max(r, 0.0) - p.item())
            loss += (-r * torch.log(torch.clamp(p, min=epsilon)))

        self._optimizer.zero_grad()
        loss.backward()
        self._optimizer.step()
        return abs(loss_error)


class SelfPlayAgent(Agent):
    def __init__(self, color, lr_=0.001):
        super().__init__(color)
        self._actions = []
        self._optimizer = torch.optim.Adam(self._model.parameters(), lr=lr_)

    def evaluate(self):
        pred = self._model(torch.from_numpy(self._in_layer))
        self._actions.append(pred)

    def get_next_move(self):
        moves = sgt.nextMoves(self._cdc, self._color)
        m = 0.
        move = None
        for i in range(0, moves.size):
            mv = moves.getMove(i)
            sgt.makeMove(self._cdc, mv)
            sgt.fillNNLayer(self._cdc, self._in_layer)
            # sgt.rebuildNN(self._cdc, self._in_layer)
            pred = self._model(torch.from_numpy(self._in_layer)).item()
            sgt.undoMove(self._cdc)
            if m < pred or move is None:
                m = pred
                move = mv

        return move

    def step(self, reward):
        rews = self._accumulate_reward(reward, len(self._actions))

        loss_error = 0.
        loss = torch.tensor([0.], requires_grad=True).double()
        for r, p in zip(rews, self._actions):
            loss_error += (reward - p.item())
            loss += (-r * torch.log(p))

        self._optimizer.zero_grad()
        loss.backward()
        self._optimizer.step()
        return abs(loss_error)

    def display_board(self):
        sgt.displayBoard(self._cdc)

    def switch_side(self):
        self._color = not self._color

    def is_draw(self):
        return sgt.isDraw(self._cdc)


import copy
import numpy as np
import torch
import torch.nn as nn
import sg_trainer_interop as sgt
import math
import os
import models
from abc import ABC

# torch.manual_seed(123)

SG_MODEL_WEIGHTS_FILE = "sg_model_weights.pt"


class Agent(ABC):
    def __init__(self, color):
        self._color = color
        self._model = models.SgModel()
        self._cdc = sgt.initCDC()
        self._in_layer = sgt.initNNInputLayer()
        self._predictions = []

    def _accumulate_reward(self, reward, gamma=0.85):
        step_count = len(self._predictions)
        assert step_count > 0
        rewards = np.zeros(step_count, dtype=np.float64)
        rewards[-1] = reward * gamma  # discard 1. immediately
        # y = 0
        for i in reversed(range(0, step_count - 1)):
            rewards[i] = rewards[i + 1] * gamma
            # y = y * gamma + rewards[i]
            # result[i] = y
        # rewards = (rewards - rewards.mean()) / (rewards.std() + 1e-8)
        return rewards

    def reset(self):
        sgt.freeCDC(self._cdc)
        self._cdc = sgt.initCDC()
        self._in_layer = sgt.initNNInputLayer()
        sgt.fillInputLayer(self._cdc, self._in_layer)
        self._predictions = []

    def checkpoint(self,):
        torch.save(self._model.state_dict(), SG_MODEL_WEIGHTS_FILE)

    def restore_state(self):
        if os.path.isfile(SG_MODEL_WEIGHTS_FILE):
            self._model.load_state_dict(torch.load(SG_MODEL_WEIGHTS_FILE, weights_only=True))

    def color(self):
        return self._color


class GamesDbAgent(Agent):
    def __init__(self, color, lr_=0.001):  # todo: color do not do anything
        super().__init__(color)
        self._optimizer = torch.optim.Adam(self._model.parameters(), lr=lr_, weight_decay=0.1)

    def make_move(self, from_sq, to_sq):
        mv = sgt.recognizeMove(self._cdc, from_sq, to_sq)
        sgt.makeMove(self._cdc, mv)

    def change_side(self):
        self._color = not self._color
        self.reset()

    def evaluate(self):
        sgt.fillInputLayer(self._cdc, self._in_layer)
        pred = self._model(torch.from_numpy(self._in_layer))
        self._predictions.append(pred)

    def step(self, reward):
        rewards = self._accumulate_reward(reward)
        assert len(rewards) == len(self._predictions)

        epsilon = 1e-11
        loss_error = 0.
        loss = torch.tensor([0.], requires_grad=True).double()
        for r, p in zip(rewards, self._predictions):
            loss_error += (r - p.item())
            loss += (-r * torch.log(torch.clamp(p, min=epsilon)))
            # loss += (-r * p)
            if torch.isinf(loss).any():
                print("loss:", loss)
                print("r:", reward, "p:", p, "log(p)", torch.log(p))
                input()

        # loss = torch.log(loss)
        self._optimizer.zero_grad()
        loss.backward()
        self._optimizer.step()
        return abs(loss_error)


class SelfPlayAgent(Agent):
    def __init__(self, color):
        super().__init__(color)

    def evaluate(self):
        sgt.fillInputLayer(self._cdc, self._in_layer)
        pred = self._model(torch.from_numpy(self._in_layer))
        self._predictions.append(pred)

    def get_next_move(self):
        moves = sgt.nextMoves(self._cdc, self._color)
        m = 0.
        move = None
        for i in range(0, moves.size):
            mv = moves.getMove(i)
            sgt.makeMove(self._cdc, mv)
            sgt.fillInputLayer(self._cdc, self._in_layer)
            pred = self._model(torch.from_numpy(self._in_layer)).item()
            sgt.undoMove(self._cdc)
            if m < pred or move is None:
                m = pred
                move = mv

        return move
        # i = np.random.randint(0, moves.size)
        # return moves.getMove(i)

    def make_move(self, mv):
        sgt.makeMove(self._cdc, mv)

    def step(self, reward):
        pass
        # cnt = len(self._predictions)
        # rewards = self._accumulate_reward(reward)
        #
        # loss_error = 0.
        # loss = torch.tensor([0.], requires_grad=True).double()
        # # for r, p in zip(rewards, self._predictiona):
        # #     loss_error += (reward - p.item())
        # #     loss += (-r * torch.log(p))
        # #     if torch.isinf(loss).any():
        # #         print("loss:", loss)
        # #         print("r:", reward, "p:", p, "log(p)", torch.log(p))
        # #         input()
        # #
        # # self._optimizer.zero_grad()
        # # loss.backward()
        # # self._optimizer.step()
        # # return abs(loss_error)
        # return 0

    def display_board(self):
        sgt.displayBoard(self._cdc)

    def switch_side(self):
        self._color = not self._color


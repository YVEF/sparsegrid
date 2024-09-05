import random

import sg_trainer_interop as sgt
import agents
import time
import chess
import chess.svg
import numpy as np


class MCTNode:
    def __init__(self, mv, is_white):
        self.value = 0.
        self.simsCount = 0
        self.winrate = 0
        self.children: list[MCTNode] = []
        self.white = is_white
        self.move: sgt.CDCMove = mv


# without mct at the moment
def run_game(agent: agents.SelfPlayAgent, opponent: agents.SelfPlayAgent, mct: MCTNode):
    players = [agent, opponent]
    if opponent.color():
        players = list(reversed(players))

    k = 0
    while True:
        curr_pl = players[k]
        k = (k + 1) % 2
        mv = curr_pl.get_next_move()
        if mv is None:
            if curr_pl.is_draw():
                return -0.5
            return -1 if curr_pl.color() == agent.color() else 1

        agent.make_move(mv)
        opponent.make_move(mv)
        if curr_pl.color() == agent.color():
            agent.evaluate()


def start(epochs, report_epochs):
    agent = agents.SelfPlayAgent(True)
    agent.restore_state()
    for ep in range(1, epochs+1):
        agent.set_side(random.choice([True, False]))
        opponent = agents.SelfPlayAgent(not agent.color())
        # restore state from previous game
        opponent.restore_state()

        mct = MCTNode(None, True)
        result = run_game(agent, opponent, mct)  # 1 - agent win, -1 - agent lose, 0 - draw
        err = agent.step(result)
        agent.checkpoint()
        agent.reset()
        print(f"epoch:{ep} loss:{err}")




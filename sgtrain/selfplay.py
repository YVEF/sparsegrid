import random

import sg_trainer_interop as sgt
import agents
import time
import chess
import chess.svg
import chess.engine
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


def start_tpo_game_(agent, engine, movetime, print_board):
    board = chess.Board()
    i = 0
    while True:
        if ((i == 0 and agent.color())
                or (i == 1 and not agent.color())):
            agent_move = agent.get_next_move()
            ucimove_str = agents.move_to_uci_str(agent_move)
            mv = chess.Move.from_uci(ucimove_str)
            if mv not in board.legal_moves:
                print("illegal move. game over")
                return -1
            board.push(mv)
            agent.make_move(agent_move)
            agent.evaluate()
        else:
            play_res = engine.play(board, chess.engine.Limit(time=movetime))
            board.push(play_res.move)
            agent.make_move_uci(play_res.move.from_square, play_res.move.to_square)

        if print_board:
            print(board)
        if board.is_checkmate():
            return 1 if i == 0 else -1
        i = (i + 1) % 2


def start_tpo_game(epochs, engine_path, movetime, print_board):
    print("start engine game")
    engine = chess.engine.SimpleEngine.popen_uci(engine_path)
    engine.configure({
        "Hash":16384,
        "Threads":1
    })
    agent = agents.SelfPlayAgent(True)
    agent.restore_state()
    for epoch in range(1, epochs+1):
        agent.set_side(random.choice([True, False]))
        result = start_tpo_game_(agent, engine, movetime, print_board)
        err = agent.step(result)
        agent.checkpoint()
        agent.reset()
        print(f'eposh:{epoch} loss:{err}')
    engine.close()


def start_selfplay(epochs, report_epochs):
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




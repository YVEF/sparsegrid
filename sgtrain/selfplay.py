import sg_trainer_interop as sgt
import agents
import time
import chess
import chess.svg


class MCTNode:
    def __init__(self, mv, is_white):
        self.value = 0.
        self.simsCount = 0
        self.winrate = 0
        self.children: list[MCTNode] = []
        self.white = is_white
        self.move: sgt.CDCMove = mv

    # def get_children(self, move):
    #     for ch in self.children:
    #         if ch.move == move:
    #             return ch
    #     return None


# now without mct
def run_game(agent: agents.SelfPlayAgent, mct: MCTNode):
    result = 0
    # i = 0
    # pboard = chess.Board()
    while True:
        mv = agent.get_next_move()
        if mv is None:
            break
        print("move:", mv.fromSq, mv.toSq, "c:", agent.color())
        agent.make_move(mv)
        agent.evaluate()
        # pmove = chess.Move(mv.fromSq, mv.toSq)
        # pboard.push(pmove)
        # chess.svg.board(pboard)
        # i = (i + 1) % 2
        agent.display_board()
        time.sleep(4)
        agent.switch_side()
    return result


def start(epochs, report_epochs):
    agent = agents.SelfPlayAgent(True)
    agent.restore_state()

    mct = MCTNode(None, True)
    result = run_game(agent, mct)  # 1 - agent1, -1 - agent2, 0 - draw
    agent.step(result)




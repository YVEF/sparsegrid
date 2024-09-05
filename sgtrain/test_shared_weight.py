import unittest
import models
import sgfiles
import os
import torch
import agents
import sg_trainer_interop as sgt


class TestSharedWeightCases(unittest.TestCase):
    def test_check2(self):
        agent = agents.GamesDbAgent(True)
        agent.restore_state()
        agent.make_move_uci(1, 18)
        agent.evaluate()
        eval_pt = agent.get_last_evaluation()
        eval_sg = sgt.getRawEvaluation(agent.get_cdc())
        self.assertAlmostEqual(eval_pt, eval_sg)


if __name__ == "__main__":
    unittest.main(verbosity=2)
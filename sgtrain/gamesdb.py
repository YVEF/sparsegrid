from time import sleep
import mct
import chess
import chess.pgn
import sg_trainer_interop as sgt
import agents
import matplotlib.pyplot as plt
import numpy as np
import os


DRAW = 0x00


def get_result(res):
    if res == "1-0":
        return 1.
    if res == "0-1":
        return -1.
    return DRAW


def run_dbgame(game, agent):
    board = game.board()
    i = 0 if agent.color() else 1
    for pgn_move in game.mainline_moves():
        agent.make_move(pgn_move.from_square, pgn_move.to_square)
        board.push(pgn_move)
        if i % 2 == 0:
            agent.evaluate()
        i += 1


def get_pgn_from_dir(dir):
    pgn_files = []
    directory = os.fsencode(dir)
    for file in os.listdir(directory):
        filename = os.fsdecode(file)
        if filename.endswith(".pgn"):
            pgn_files.append(os.path.join(dir, filename))

    return pgn_files


def start(max_epochs, report_epochs):
    agent = agents.GamesDbAgent(True)
    agent.restore_state()

    plot_loss_errors = []
    plot_epoch_error = 0.
    plot_epoch_errors = []
    epoch = 0
    pgn_files = get_pgn_from_dir("/home/iaroslav/Downloads/csvn")
    while epoch < max_epochs:
        game_count = 0
        for pgn_file in pgn_files:
            print("file:", pgn_file)
            pgn = open(pgn_file)
            # pgn = open("/home/iaroslav/Downloads/csvn/A01.commented.[15384].pgn")
            game = chess.pgn.read_game(pgn)
            for i in range(0, 2):
                while game is not None:
                    result = get_result(game.headers["Result"])
                    if result == DRAW:
                        game = chess.pgn.read_game(pgn)
                        continue

                    game_count += 1
                    run_dbgame(game, agent)
                    loss_err = agent.step(result if agent.color() else -result)
                    plot_loss_errors.append(loss_err)
                    plot_epoch_error += loss_err
                    agent.reset()
                    game = chess.pgn.read_game(pgn)
                agent.change_side()

        epoch += 1
        plot_epoch_errors.append(plot_epoch_error)
        print("Epoch:", epoch, "error:", plot_epoch_error)
        plot_epoch_error = 0.
        if epoch % report_epochs == 0:
            print("check:", plot_epoch_errors)
            plt.plot(plot_epoch_errors)
            plt.title("Loss functions")
            plt.show()
            plot_loss_errors = []
            plot_epoch_errors = []

    agent.checkpoint()


from time import sleep
import chess
import chess.pgn
import sg_trainer_interop as sgt
import torch
import agents
import matplotlib.pyplot as plt
import numpy as np
import os
import alive_progress as ap


DRAW = 0x00
WIN = 1.
LOSE = -1.

# torch.manual_seed(12)


def get_result(res):
    if res == "1-0":
        return WIN
    if res == "0-1":
        return LOSE
    return DRAW


def run_dbgame(game, agent):
    i = 0 if agent.color() else 1
    for pgn_move in game.mainline_moves():
        agent.make_move_uci(pgn_move.from_square, pgn_move.to_square)
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


def compute_games_number(pgn_file):
    pgn = open(pgn_file)
    number = 0
    game = chess.pgn.read_game(pgn)
    while game is not None:
        result = get_result(game.headers["Result"])
        if result == DRAW:
            game = chess.pgn.read_game(pgn)
            continue

        number += 1
        game = chess.pgn.read_game(pgn)
    pgn.close()
    return number


def start(pgn_dir, max_epochs, report_epochs):
    agent = agents.GamesDbAgent(True)
    agent.restore_state()

    plot_epoch_error = 0.
    plot_epoch_errors = []
    epoch = 1
    pgn_files = get_pgn_from_dir(pgn_dir)
    while epoch <= max_epochs:
        for pgn_file in pgn_files:
            print("file:", pgn_file)
            games_number = compute_games_number(pgn_file) * 2
            pgn = open(pgn_file)
            game = chess.pgn.read_game(pgn)
            games_processed = 0
            with ap.alive_bar(games_number) as bar:
                while game is not None:
                    result = get_result(game.headers["Result"])
                    if result == DRAW:
                        game = chess.pgn.read_game(pgn)
                        continue

                    for i in range(0, 2):
                        run_dbgame(game, agent)
                        loss_err = agent.step(result if agent.color() else -result)
                        plot_epoch_error += loss_err
                        agent.switch_side()
                        games_processed += 1
                        bar()

                    game = chess.pgn.read_game(pgn)

            pgn.close()
            agent.checkpoint()

        plot_epoch_errors.append(plot_epoch_error)
        print("Epoch:", epoch, "error:", plot_epoch_error)
        plot_epoch_error = 0.
        if report_epochs is not None and epoch % report_epochs == 0:
            print("check:", plot_epoch_errors)
            plt.plot(plot_epoch_errors)
            plt.title("Loss functions")
            plt.show()
            plot_epoch_errors = []

        epoch += 1


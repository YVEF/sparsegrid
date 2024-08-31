import gamesdb
import selfplay
import argparse


# Here is 2 types of training on the moment:
# 1) Games db: huge games db train the model predict the full precise trajectory
#       - Backpropagation and optimization of the fly without actuall game tree
# 2) Game simulation: play with various agent (e.g. self-playing, Stockfish etc...)
#       - Here the MCTS is actully in use


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Run SparseGrid trainer")
    parser.add_argument("--training",
                        choices=['g', 's'],
                        # type=ascii,
                        required=True,
                        help="Type of training")

    parser.add_argument("--epochs",
                        type=int,
                        required=True,
                        help="Number of training epochs")

    parser.add_argument("--plot_ep", nargs='?',
                        type=int,
                        required=False,
                        help="Epochs of interval of plotting the loss graph")

    parser.add_argument("--pgn_dir", nargs='?',
                        required=False,
                        help="PGN DB directory")

    args = parser.parse_args()
    epochs = args.epochs
    plot_epochs = max(2, epochs/300) if args.plot_ep is None else args.plot_ep
    if args.training == 'g':
        if args.pgn_dir is None or args.pgn_dir == "":
            print("Missing pgn directory")
            exit(1)
        gamesdb.start(args.pgn_dir, epochs, plot_epochs)
    else:
        selfplay.start(epochs, plot_epochs)


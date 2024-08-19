import gamesdb
import selfplay
import argparse
import converters


# Here is 2 types of training on the moment:
# 1) Games db: huge games db train the model predict the full precise trajectory
#       - Backpropagation and optimization of the fly without actuall game tree
# 2) Game simulation: play with various agent (e.g. self-playing, Stockfish etc...)
#       - Here the MCTS is actully in use


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Run SparseGrid trainer")
    parser.add_argument("--training",
                        choices=['g', 's'],
                        help="Type of training")

    parser.add_argument("--epochs",
                        type=int,
                        help="Number of training epochs")

    parser.add_argument("--plot_ep", nargs='?',
                        type=int,
                        default=None,
                        required=False,
                        help="Epochs of interval of plotting the loss graph")

    parser.add_argument("--pgn_dir", nargs='?',
                        required=False,
                        help="PGN DB directory")

    parser.add_argument("--convert_weights",
                        action="store_true",
                        help="Convert weights into sg format")

    args = parser.parse_args()
    if args.convert_weights:
        res = converters.sg_convert_weights()
        exit(res)

    epochs = args.epochs
    plot_epochs = args.plot_ep
    if args.training == 'g':
        if args.pgn_dir is None or args.pgn_dir == "":
            print("Missing pgn directory")
            exit(1)
        gamesdb.start(args.pgn_dir, epochs, plot_epochs)
    else:
        selfplay.start(epochs, plot_epochs)


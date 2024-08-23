from time import sleep
import mct
import chess
import chess.pgn
import sg_trainer_interop as sgt


def convert(mv: sgt.CDCMove):
    res = mct.CDCMove()
    res.fromSq = mv.fromSq
    res.toSq = mv.toSq
    res.castling = mv.castling
    res.isEnpass = mv.isEnpass
    return res


def print_tree_line(node):
    c = node
    while True:
        if len(c.children) == 0:
            break
        mv = c.children[0].move
        print(mv.fromSq, mv.toSq, mv.castling, "sim count:", c.simsCount, "winrate:", c.winrate)
        c = c.children[0]


def get_result(res):
    if res == "1-0":
        return 0x01
    if res == "0-1":
        return 0x02
    return 0x00


DRAW = 0x00


def run_human_test():
    game_count = 0
    pgn = open("/home/iaroslav/Downloads/csvn/csvn2006.pgn")
    game = chess.pgn.read_game(pgn)
    node = mct.MCTNode(None, True)
    while game is not None:
        game_result = get_result(game.headers["Result"])
        if game_result != DRAW:
            game_count = game_count + 1
            simulate(game, node, game_result)

        game = chess.pgn.read_game(pgn)


def simulate(game, node: mct.MCTNode, result):
    """ Result = 0x01 - white, 0x02 - black, 0x00 - draw """
    assert node.white
    print(game)
    print()
    cdc = sgt.initCDC(True)
    board = game.board()
    currNode: mct.MCTNode = node
    if result == 0x01:
        currNode.winrate = currNode.winrate + 1

    for pgnMove in game.mainline_moves():
        print("pgn:", pgnMove.from_square, pgnMove.to_square)
        sgMove: mct.CDCMove = sgt.recognizeMove(cdc, pgnMove.from_square, pgnMove.to_square)
        cdcMove = convert(sgMove)
        print("sg: ", cdcMove)

        if cdcMove.castling:
            print("castling", cdcMove.castling)
        elif cdcMove.isEnpass:
            print("enpass", cdcMove.isEnpass)
        else:
            if cdcMove.fromSq != pgnMove.from_square or cdcMove.toSq != pgnMove.to_square:
                print("Error here!!!!")
                raise Exception("lol???")

        sgt.makeMove(cdc, sgMove)
        board.push(pgnMove)
        # if game_count == 2:
        #     print("here")
        child = currNode.get_children(cdcMove)

        if child is None:
            # if game_count == 2:
            #     print(cdcMove)
            #     sleep(10)

            newnode = mct.MCTNode(cdcMove, not currNode.white)
            currNode.children.append(newnode)
            currNode = newnode
        else:
            # print("FOUND!!!")
            currNode = child

        currNode.simsCount = currNode.simsCount + 1
        if (currNode.white and result == 0x01) or (not currNode.white and result == 0x02):
            currNode.winrate = currNode.winrate + 1

    print("game over")
    sgt.freeCDC(cdc)

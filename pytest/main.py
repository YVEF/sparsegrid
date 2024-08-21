from ctypes import cdll
import sg_trainer_interop as sgt


def run():
    cdc = sgt.initCDC(True)
    sgt.welcome(cdc)
    moveCollection = sgt.nextMoves(cdc, True)
    print(moveCollection.size)

    for i in range(moveCollection.size):
        mv = moveCollection.getMove(i)
        print(mv.fromSq, mv.toSq)

    inputLayer = sgt.initNNInputLayer()
    print(inputLayer)


    return


if __name__ == "__main__":
    run()

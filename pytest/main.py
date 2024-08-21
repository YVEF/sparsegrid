from ctypes import cdll


def run():
    lib = cdll.LoadLibrary('../cmake-build-debug/src/libsparsegridcoreshared.so')
    lib.printEngineHello(5)
    cdc = lib.initCDC(True)
    lib.welcome(cdc)
    moveCollection = lib.nextMoves(cdc, True)
    # print(moveCollection.size())
    return


if __name__ == "__main__":
    run()

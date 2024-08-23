class CDCMove:
    def __init__(self):
        self.fromSq = 0
        self.toSq = 0
        self.castling = 0
        self.isEnpass = False

    def __repr__(self):
        return f'from: {self.fromSq} to: {self.toSq} c: {self.castling} e: {self.isEnpass}'

    def __eq__(self, other):
        if isinstance(other, CDCMove):
            return (self.fromSq == other.fromSq
                    and self.toSq == other.toSq
                    and self.castling == other.castling
                    and self.isEnpass == other.isEnpass)
        return False


class MCTNode:
    def __init__(self, mv, is_white):
        self.value = 0.
        self.simsCount = 0
        self.winrate = 0
        self.children: list[MCTNode] = []
        self.white = True
        self.move: CDCMove = mv

    def get_children(self, move):
        # print("get_children")
        # if move.fromSq == 12 and move.toSq == 28:
        #     print("AGA")
        #     print(len(self.children))
        #     if len(self.children) > 0:
        #         print(self.children[0].move == move)
        #         print("doo", self.children[0].move)
        #         input()

        for ch in self.children:
            if ch.move == move:
                return ch
        return None

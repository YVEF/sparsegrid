# class CDCMove:
#     def __init__(self):
#         self.fromSq = 0
#         self.toSq = 0
#         self.castling = 0
#         self.isEnpass = False
#
#     def __repr__(self):
#         return f'from: {self.fromSq} to: {self.toSq} c: {self.castling} e: {self.isEnpass}'
#
#     def __eq__(self, other):
#         if isinstance(other, CDCMove):
#             return (self.fromSq == other.fromSq
#                     and self.toSq == other.toSq
#                     and self.castling == other.castling
#                     and self.isEnpass == other.isEnpass)
#         return False
#

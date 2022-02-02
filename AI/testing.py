import numpy as np
import chess

fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
board = chess.Board(fen)

def SquareSetToBitboard(square):
    return list(map(int,str(square).replace(".","0").split()))

lookup = {"K":0,"Q":1,"k":2,"q":3}
#Converts the xfen format to more useful castling information
#xfen is the old notation which doesn't work for chess360 but works better for normal chess
def XFenToBits(xfen):
    ans = [0,0,0,0]
    for i in xfen:
        ans[lookup[i]] = 1
    return ans

print(board.castling_xfen())

pieces = [chess.PAWN,chess.BISHOP,chess.KING,chess.QUEEN,chess.ROOK]
colours = [chess.WHITE,chess.BLACK]

def GetBitboard(board):
    bitboard = []
    for piece in pieces:
        for colour in colours:
            bitboard.extend(SquareSetToBitboard(board.pieces(piece,colour)))
    bitboard.extend(XFenToBits(board.castling_xfen()))
    return bitboard

print(GetBitboard(board))
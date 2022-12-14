import tensorflow as tf
from keras.utils.vis_utils import plot_model
from keras.engine import input_layer
from keras.models import Sequential
from keras.layers import Dense
import keras
import numpy as np
import chess



model = Sequential()
model.add(Dense(20,input_dim=772,activation="relu"))
#model.add(Conv1D(filters=256, kernel_size=5, padding='same', activation='relu', input_dim=644))
model.add(Dense(20,activation="relu"))
model.add(Dense(20,activation="relu"))
model.add(Dense(20,activation="relu"))
model.add(Dense(20,activation="relu"))
model.add(Dense(20,activation="relu"))
model.add(Dense(20,activation="relu"))
model.add(Dense(1,activation="sigmoid"))
model.compile(optimizer="Adam", loss="mean_squared_error", metrics=["mae"])

checkpoint_path = "Model/weights.ckpt"


# model.load_weights(checkpoint_path)

model = keras.models.load_model('model')


pieces = [chess.PAWN,chess.BISHOP,chess.KING,chess.QUEEN,chess.ROOK,chess.KNIGHT]
colours = [chess.WHITE,chess.BLACK]

lookup = {"K":0,"Q":1,"k":2,"q":3}
#Converts the xfen format to more useful castling information
#xfen is the old notation which doesn't work for chess360 but works better for normal chess
def xfen_to_bits(xfen):
    if xfen == "-":
        return [0,0,0,0]
    ans = [0,0,0,0]
    for i in xfen:
        ans[lookup[i]] = 1
    return ans

def square_set_to_bitboard(square):
    return list(map(int,str(square).replace(".","0").split()))

def get_bitboard(board):
    #Need to flip endianness for black to play
    bitboard = []
    for piece in pieces:
        for colour in colours:
            bitboard.extend(square_set_to_bitboard(board.pieces(piece,colour)))
    bitboard.extend(xfen_to_bits(board.castling_xfen()))
    return bitboard


fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
board = chess.Board(fen)

print("".join(map(str,get_bitboard(board))))
ans = model.predict([get_bitboard(board)])

print(ans[0])
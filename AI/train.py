import multiprocessing
from tkinter import filedialog
from turtle import Turtle
from keras.utils.vis_utils import plot_model
from keras.engine import input_layer
from keras.models import Sequential
from keras.layers import Dense,Conv2D,Conv1D
import chess
import numpy
import json
import random

def get_data_from_file(name):
    with open(name,"r") as data:
        return json.loads(data.read())

def square_set_to_bitboard(square):
    return list(map(int,str(square).replace(".","0").split()))

def convert_cp_to_prob(cp):
    return 1/(1 + 10**(-cp/400))


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

pieces = [chess.PAWN,chess.BISHOP,chess.KING,chess.QUEEN,chess.ROOK,chess.KNIGHT]
colours = [chess.WHITE,chess.BLACK]

def get_bitboard(board):
    #Need to flip endianness for black to play
    bitboard = []
    for piece in pieces:
        for colour in colours:
            bitboard.extend(square_set_to_bitboard(board.pieces(piece,colour)))
    bitboard.extend(xfen_to_bits(board.castling_xfen()))
    return bitboard

def get_training_data(file_name):
    data_set = get_data_from_file(file_name)
    inputs = []
    outputs = []

    for i in data_set.keys():
        board = chess.Board(i)
        inputs.append(get_bitboard(board))
        outputs.append(data_set[i])


    #Remove all the mate positions which since the code should be able to evaluate these anyway
    #Also converts from cp to probabilities
    inputs = [inputs[i] for i in range(len(inputs)) if outputs[i] != None]
    outputs = [convert_cp_to_prob(outputs[i]) for i in range(len(outputs)) if outputs[i] != None]

    i = numpy.asarray(inputs).astype(numpy.int32)
    o = numpy.asarray(outputs).astype(numpy.float32)

    return i,o

def batch_generator(file_names):
    for file in file_names:
        inputs,outputs = get_training_data(file)
        #Shuffle the lists together
        combined = list(zip(inputs,outputs))
        random.shuffle(combined)
        inputs,outputs = zip(*combined)
        
        
        

#Defining networkprint(model.predict([i[0:1000]]))


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


gen = batch_generator(["Data/Positions/batch-out" + str(i) + ".json" for i in range(0,73)])
print("here")
# i_0,o_0 = get_training_data("Data/Positions/batch-out0.json")
# i_1,o_1 = get_training_data("Data/Positions/batch-out1.json")
# model.fit(numpy.concatenate((i_0,i_1)),numpy.concatenate((o_0,o_1)),use_multiprocessing=True,epochs=10)
i,o = get_training_data("Data/Positions/batch-out" + str(0) + ".json")
for j in range(1,73):
    i_temp,o_temp = get_training_data("Data/Positions/batch-out" + str(j) + ".json")
    i = numpy.concatenate((i,i_temp))
    o = numpy.concatenate((o,o_temp))
    print(j)


#model.fit(i,o,use_multiprocessing=True,epochs=10)

#i, o = get_training_data("Data/Positions/batch-out70.json")

model.evaluate(i,o,use_multiprocessing=True)
history = model.fit(
                    gen,
                    steps_per_epoch=20,
                    epochs=3,
                    max_queue_size=15,
                    use_multiprocessing=True,
                    )

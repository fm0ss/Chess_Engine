def unpack(move):
    origin = (move >> 10)
    destination = ((move & 0x3f0) >> 4)
    print(convert_to_notation(origin),convert_to_notation(destination))


lookup = {0:"a",1:"b",2:"c",3:"d",4:"e",5:"f",6:"g",7:"h"}
def convert_to_notation(pos):
    string = ""
    string += lookup[pos % 8]
    string += str(pos // 8 + 1)
    
    return string


moves = [
11696,
56032,
12736,
59008,
3376,
53952,
2224,
52912,
6480,
63328,
10528,
62256,
19776,
56048,
20784,
52896,
19984,
64320
]

for i in moves:
    unpack(i)


def pack(origin,destination):
    move = 0
    move += (destination << 4)
    move += (origin << 10)
    print(move)
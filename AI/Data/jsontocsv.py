import json
import random

def convert_file_to_csv(filename):
    with open(filename,"r") as data:
        d = json.loads(data.read())
    
    with open(filename.rstrip(".json") + ".csv","w") as csvfile:
        #Shuffle them to ensure randomness
        keys = list(d.keys())
        random.shuffle(keys)
        for i in keys:
            csvfile.write("{},{}\n".format(i,d[i]))


for i in range(0,74):
    convert_file_to_csv("Positions/batch-out" + str(i) + ".json")
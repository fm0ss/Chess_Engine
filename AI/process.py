from turtle import pos
import re
import json
import chess
import chess.engine


engine = chess.engine.SimpleEngine.popen_uci("stockfish")
positions = {}

def stockfish_evaluation(board : chess.Board) -> int:
    result = engine.analyse(board, chess.engine.Limit(depth=6))
    return result["score"].white().score()


def ExtractGamesFromFile(FileName : str) -> list:
    with open(FileName) as file:
        data = file.read()
        games = re.findall('\n1\.(?:.*?)(?:1\-0|0\-1|1\/2\-1\/2)',data)
        for i in range(len(games)):
            games[i] = re.sub('{(?:.*?)}','',games[i])
            games[i] = re.split("\d+\.",games[i])
            #games[i] = re.sub('\d+\.','.',games[i])
            #games[i] = games[i].split(".")
            for j in range(len(games[i])):
                games[i][j] = games[i][j].split()
            #Remove empty string at the start, and result at the end
            games[i][-1].pop(-1)
            games[i].pop(0)
        return games
    
def ProcessPosition(board):
    fen = board.fen()
    if fen not in positions.keys():
        eval = stockfish_evaluation(board)
        positions[fen] = eval


def ProcessGame(game):
    fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    board = chess.Board(fen)
    for i in game:
        for move in i:
            if board.turn == chess.WHITE:
                ProcessPosition(board)
            else:
                ProcessPosition(board.mirror())

            #Make the move
            try:
                board.push_san(move)
            except ValueError:
                pass
            
            #print(ExtractGamesFromFile("Data/games.pgn"))

#print(ExtractGamesFromFile("Data/games.pgn")[4])
batch_number = 0
for game in ExtractGamesFromFile("lichess_db_standard_rated_2013-01.pgn"):
    #print(game)
    if len(positions) >= 100000:
        with open("batch-out" + str(batch_number) + ".json","w") as batch:
            batch.write(json.dumps(positions))
            print("Completed batch",batch_number)
            batch_number += 1
            count = 0
            positions = {}
    ProcessGame(game)

with open("Data/batch-out" + str(batch_number) + ".json","w") as batch:
    batch.write(json.dumps(positions))

#chess.engine creates its own process so needs to be explicitly told to quit otherwise this program will not terminate
engine.quit()
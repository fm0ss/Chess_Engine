import re
import json
import chess
import chess.engine


engine = chess.engine.SimpleEngine.popen_uci("stockfish")
positions = {}

#Gives an evaluation from stockfish
def stockfish_evaluation(board : chess.Board) -> int:
    result = engine.analyse(board, chess.engine.Limit(depth=6))
    return result["score"].white().score()

#Uses stockfish to evaluate a position  
def process_position(board : chess.Board,positions : dict) -> None:
    fen = board.fen()
    if fen not in positions.keys():
        eval = stockfish_evaluation(board)
        positions[fen] = eval


#Given a file this will return a list of all the games in it
def extract_games_from_file(FileName : str) -> list:
    with open(FileName) as file:
        data = file.read()
        games = re.findall('\n1\.(?:.*?)(?:1\-0|0\-1|1\/2\-1\/2)',data)
        #Remove games with evaluations
        games = [i for i in games if "{" not in i]
        for i in range(len(games)):
            #Split on digit separators
            games[i] = re.split("\d+\.",games[i])

            for j in range(len(games[i])):
                games[i][j] = games[i][j].split()
            #Remove empty string at the start, and result at the end
            games[i][-1].pop(-1)
            games[i].pop(0)
        return games

#Runs through a game and evaluates each position
def process_game(game,positions):
    fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    board = chess.Board(fen)
    for i in game:
        for move in i:
            if board.turn == chess.WHITE:
                process_position(board,positions)
            else:
                process_position(board.mirror(),positions)

            #Make the move
            #Some of the pgn games have a slightly different format which we pick up wrong
            #If we have one of these games we should just continue to the next game
            #This is a minority of games
            try:
                board.push_san(move)
            except ValueError:
                break
            
            #print(ExtractGamesFromFile("Data/games.pgn"))

#print(ExtractGamesFromFile("Data/games.pgn")[4])
batch_number = 0
for game in extract_games_from_file("lichess_db_standard_rated_2013-01.pgn"):
    #print(game)
    if len(positions) >= 100000:
        with open("batch-out" + str(batch_number) + ".json","w") as batch:
            batch.write(json.dumps(positions))
            print("Completed batch",batch_number)
            batch_number += 1
            count = 0
            positions = {}
    process_game(game,positions)

with open("Data/batch-out" + str(batch_number) + ".json","w") as batch:
    batch.write(json.dumps(positions))

#chess.engine creates its own process so needs to be explicitly told to quit otherwise this program will not terminate
engine.quit()
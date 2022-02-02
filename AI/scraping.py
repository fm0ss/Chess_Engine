from stockfish import Stockfish
import re
import requests
from bs4 import BeautifulSoup
import time


def GetNames(start,end):
    for page_number in range(start,end + 1):
        page = requests.get("https://www.chess.com/games?page=" + str(page_number))
        soup = BeautifulSoup(page.text)
        #Get all links which are correct format
        links = list(set([link['href'] for link in soup.find_all('a',href=True) if "https://www.chess.com/games/" in link['href'] and 'view' not in link['href']]))
        return links

def GetGames(game_numbers):
    return requests.post("https://www.chess.com/games/downloadPgn?game_ids=" + ",".join(game_numbers)).text

def GetNumbers(link):
    page = requests.get(link)
    soup = BeautifulSoup(page.text)
    numbers = list(set([link['href'].lstrip("https://www.chess.com/games/view/") for link in soup.find_all('a',href=True) if "https://www.chess.com/games/" in link['href'] and 'view' in link['href']]))
    return numbers

#ExtractGamesFromFile("Data/Garry-Kasparov_vs_Anatoly-Karpov_2021.08.24.pgn")

#print(ExtractGamesFromFile("Data/master_games.pgn"))

#GetNames(1,1)

with open("Data/games.pgn","a") as games:
    for i in GetNames(1,10):
        time.sleep(10)
        numbers = GetNumbers(i)
        games.write(GetGames(numbers))

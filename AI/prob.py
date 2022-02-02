import math

def stirling(n):
    return math.sqrt(2*math.pi*n)*(n/math.e)**n

stirling(1.1*10**77)
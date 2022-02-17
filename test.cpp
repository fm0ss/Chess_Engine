#include<iostream>
#include<stdint.h>
#include"magic.hpp"
#include <chrono>
#include"string.h"


int test_func(uint64_t x){
    int c = 0;
    while(x != 0){
        x >>= 1;
        c++;
    }
    return c;
}

uint64_t isolate_lsb(uint64_t x){
    return x & -x;
}

int DeBruijn(uint64_t x){
    const unsigned int convert[64] = {
        0,  1,  2, 53,  3,  7, 54, 27,
        4, 38, 41,  8, 34, 55, 48, 28,
        62,  5, 39, 46, 44, 42, 22,  9,
        24, 35, 59, 56, 49, 18, 29, 11,
        63, 52,  6, 26, 37, 40, 33, 47,
        61, 45, 43, 21, 23, 58, 17, 10,
        51, 25, 36, 32, 60, 20, 57, 16,
        50, 31, 19, 15, 30, 14, 13, 12
    };
    uint64_t DEBRUIJN_CONSTANT = 0x022fdd63cc95386d;
    x = isolate_lsb(x);
    x *= DEBRUIJN_CONSTANT;
    x >>= 58;
    return convert[x];
}

int main(){
    // std::cout << ((2147483656 * RMagic[0]) >> (64 - RBits[0])) << std::endl;
    // std::cout << (512 * BMagic[0]) << std::endl;
    std::cout << ffsl(1ULL << 63);
    return 0;
}

void LoopThroughInt(uint64_t integer){
    //start at position 0
    int pos = 0;
    int lsb;
    while(integer != 0){
        lsb = __builtin_ffs(integer);
        pos += lsb;
        //Do something with pos here

        //Now the integer is shifted
        integer >>= lsb + 1;
    }
}
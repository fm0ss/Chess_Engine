#include<stdint.h>
#include<iostream>
#include <strings.h>


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

const uint64_t DEBRUIJN_CONSTANT = 0x022fdd63cc95386d;
int DeBruijn(uint64_t);

uint64_t isolate_lsb(uint64_t x){
    return x & -x;
}

int Get_Lsb(uint64_t x){
    return ffsl(x);
}

int DeBruijn(uint64_t x){
    x = isolate_lsb(x);
    x *= DEBRUIJN_CONSTANT;
    x >>= 58;
    return convert[x];
}

int main(){
    uint64_t x = 1;
    //Sum is used here to stop the compiler just noticing that the program doesn't do anything
    uint64_t sum = 0;
    for (int j = 0; j < 100000; j++){
        for(int i = 0; i < 64; i++){
            sum += Get_Lsb(x << i);
        }
    }
    std::cout << sum << std::endl;
}

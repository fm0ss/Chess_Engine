#include"DeBruijn.hpp"

uint64_t isolate_lsb(int64_t x){
    return x & -x;
}

int debruijn(uint64_t x){
    x = isolate_lsb(x);
    x *= DEBRUIJN_CONSTANT;
    x >>= 58;
    return convert[x];
}


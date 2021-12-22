#include<iostream>
#include<stdint.h>
#include"magic.hpp"

int main(){
    std::cout << ((2147483656 * RMagic[0]) >> (64 - RBits[0])) << std::endl;
    std::cout << (512 * BMagic[0]) << std::endl;
    return 0;
}
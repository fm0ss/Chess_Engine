#include<stdint.h>
#include<unordered_map>

typedef struct Magics Magics;

struct Magics{
    const uint64_t* r_magic;
    const uint64_t* b_magic;
    const int* r_bits;
    const int* b_bits;
};

Magics* Get_Magics();

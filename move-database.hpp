#include<stdint.h>
#include<iostream>
#include<vector>
#include<unordered_map>
#include"magic.hpp"

typedef struct MoveData MoveData;

struct MoveData{
  std::unordered_map<uint_fast16_t,uint64_t> r_moves[64];
  std::unordered_map<uint_fast16_t,uint64_t> b_moves[64];
  uint64_t r_mask[64];
  uint64_t b_mask[64];
  uint64_t kn_moves[64];
  uint64_t ki_moves[64];
};

MoveData* get_move_data(Magics*);
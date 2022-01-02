#include<unordered_map>
#include<stdint.h>

typedef struct MoveData MoveData;

struct MoveData{
  std::unordered_map<uint_fast16_t,uint64_t> rMoves[64];
  std::unordered_map<uint_fast16_t,uint64_t> bMoves[64];
  uint64_t rMask[64];
  uint64_t bMask[64];
  uint64_t knMoves[64];
  uint64_t kiMoves[64];
};

MoveData* get_move_data();
#include"move-database.hpp"

uint64_t cartesian_to_int(int x, int y){
  //This is only needed for the knight generation
  //EDIT also for king move generation
  if (x < 8 && x >= 0 && y < 8 && y >= 0){
    uint64_t result = 1;
    result <<= (y * 8 + x);
    return result;
  }
  return 0;
}

//Maps the integers onto the 64 bit integers in the specified directions
uint64_t add_ones_bishop(int ld, int lu, int rd, int ru, int x, int y){
  uint64_t result = 0;
  //Go from 1 to 6 since 0 is starting square and 7 is too far
  for (int i = 1; i < 7; i++){
    if (((ld >> i) & 1ULL) == 1ULL){
      result |= cartesian_to_int(x - i, y - i);
    }
    if (((lu >> i) & 1ULL) == 1ULL){
      result |= cartesian_to_int(x - i, y + i);
    }
    if (((rd >> i) & 1ULL) == 1ULL){
      result |= cartesian_to_int(x + i, y - i);
    }
    if (((ru >> i) & 1ULL) == 1ULL){
      result |= cartesian_to_int(x + i, y + i);
    }
  }
  return result;
}

//Maps the integers onto the 64 bit integers in the specified directions
uint64_t add_ones_rook(int l,int r,int d,int u,int x,int y){
  uint64_t result = 0;
  //Go from 1 to 6 since 0 is starting square and 7 is too far
  for (int i = 1; i < 7; i++){
      if (((l >> i) & 1ULL) == 1ULL){
        result |= cartesian_to_int(x - i,y);
      }
      if (((r >> i) & 1ULL) == 1ULL){
        result |= cartesian_to_int(x + i,y);
      }
      if (((d >> i) & 1ULL) == 1ULL){
        result |= cartesian_to_int(x ,y - i);
      }
      if (((u >> i) & 1ULL) == 1ULL){
        result |= cartesian_to_int(x ,y + i);
      }
  }
  return result;
}

uint64_t rook_attack(uint64_t blocker,int x,int y){
  //Sends out rays until we hit something in each direction
  uint64_t result = 0;
  for(int i = 1; i < x + 1; i++){
    result |= cartesian_to_int(x - i , y);
    if (blocker & cartesian_to_int(x - i, y)){
      break;
    }
  }
  for(int i = 1; i < (8 - x); i++){
    result |= cartesian_to_int(x + i , y);
    if (blocker & cartesian_to_int(x + i, y)){
      break;
    }
  }
  for(int i = 1; i < y + 1; i++){
    result |= cartesian_to_int(x , y - i);
    if (blocker & cartesian_to_int(x, y - i)){
      break;
    }
  }
  for(int i = 1; i < (8 - y); i++){
    result |= cartesian_to_int(x , y + i);
    if (blocker & cartesian_to_int(x, y + i)){
      break;
    }
  }
  return result;
}

uint64_t bishop_attack(uint64_t blocker, int x, int y){
  uint64_t result = 0;
  for (int i = 1; i < std::min(x,y) + 1;i++){
    result |= cartesian_to_int(x - i, y - i);
    if (blocker & cartesian_to_int(x - i, y - i)){
      break;
    }
  }
  for (int i = 1; i < std::min(7 - x,y) + 1;i++){
    result |= cartesian_to_int(x + i, y - i);
    if (blocker & cartesian_to_int(x + i, y - i)){
      break;
    }
  }
  for (int i = 1; i < std::min(x,7 - y) + 1;i++){
    result |= cartesian_to_int(x - i, y + i);
    if (blocker & cartesian_to_int(x - i, y + i)){
      break;
    }
  }
  for (int i = 1; i < std::min(7 - x,7 - y) + 1;i++){
    result |= cartesian_to_int(x + i, y + i);
    if (blocker & cartesian_to_int(x + i, y + i)){
      break;
    }
  }
  return result;
}

//Populate the hash map for rooks for all possible positions
void add_rook_moves(std::unordered_map<uint_fast16_t,uint64_t>* rMoves,int x,int y,Magics* MagicNumbers){
  int position = y * 8 + x;
  uint64_t mask;
  uint_fast16_t key;
  int count = 0;
  //Loop through the left,right,up down integers
  for (int l = 0; l < (1 << x);l++){
      for (int r = 0; r < (1 << (7 - x));r++){
          for (int d = 0; d < (1 << y);d++){
              for (int u = 0; u < (1 << (7 - y));u++){
                  //Create key
                  //Overlay integers onto one 64 bit integer
                  mask = add_ones_rook(l,r,d,u,x,y);
                  key = (mask * MagicNumbers->r_magic[position]) >> (64 - MagicNumbers->r_bits[position]);
                  //Add to the hashMap
                  if (rMoves[position].find(key) == rMoves[position].end()){
                    rMoves[position][key] = rook_attack(mask,x,y);
                  }
                  else{
                    if (rMoves[position][key] != rook_attack(mask,x,y)){
                      count++;
                    }
                  }
                }
              }
          }
      }
    //std::cout << count;
}

void add_bishop_moves(std::unordered_map<uint_fast16_t,uint64_t>* bMoves,int x,int y,Magics* MagicNumbers){
  int position = y * 8 + x;
  uint64_t mask;
  uint_fast16_t key;
  int count = 0;
  for (int ld = 0; ld < (1 << std::min(x,y));ld++){
    for (int lu = 0; lu < (1 << std::min(x,7- y));lu++){
      for (int rd = 0; rd < (1 << std::min(7 - x, y));rd++){
        for (int ru = 0; ru < (1 << std::min(7 - x, 7 - y));ru++){
          mask = add_ones_bishop(ld,lu,rd,ru,x,y);
          key = (mask * MagicNumbers->b_magic[position]) >> (64 - MagicNumbers->b_bits[position]); 
          if (bMoves[position].find(key) == bMoves[position].end()){
            bMoves[position][key] = bishop_attack(mask,x,y);
          }
          else{
            if (bMoves[position][key] != bishop_attack(mask,x,y)){
              count++;
            }
          }
        }
      }
    }
  }
  //std::cout << count;
}

// uint64_t generate_position_rook(int x, int y){
//   int64_t sum = 0;
//   for(int i = 1; i < x; i++){
//     sum |= carte64 - RBits[y * 8 + x]sian_to_int(x - i, y);
//   }
//   for(int i = 1; i < (7 - x); i++){
//     sum |= cartesian_to_int(x + i ,y);
//   })
// }

//Generates knight attack from a position
uint64_t knight_attack(int x, int y){
  uint64_t result = 0;
  result |= cartesian_to_int(x + 2, y + 1);
  result |= cartesian_to_int(x + 2, y - 1);
  result |= cartesian_to_int(x - 2, y + 1);
  result |= cartesian_to_int(x - 2, y - 1);
  result |= cartesian_to_int(x + 1, y + 2);
  result |= cartesian_to_int(x + 1, y - 2);
  result |= cartesian_to_int(x - 1, y + 2);
  result |= cartesian_to_int(x - 1, y - 2);
  return result;
}

uint64_t king_attack(int x, int y){
  uint64_t result = 0;
  result |= cartesian_to_int(x + 1, y);
  result |= cartesian_to_int(x + 1, y + 1);
  result |= cartesian_to_int(x + 1, y - 1);
  result |= cartesian_to_int(x - 1, y);
  result |= cartesian_to_int(x - 1, y + 1);
  result |= cartesian_to_int(x - 1, y - 1);
  result |= cartesian_to_int(x, y + 1);
  result |= cartesian_to_int(x, y - 1);
  return result;
}

uint64_t rook_mask_gen(int x,int y){
  //Sends out rays until we hit something in each direction
  uint64_t result = 0;
  for(int i = 1; i < x; i++){
    result |= cartesian_to_int(x - i , y);
  }
  for(int i = 1; i < (7 - x); i++){
    result |= cartesian_to_int(x + i , y);
  }
  for(int i = 1; i < y; i++){
    result |= cartesian_to_int(x , y - i);
  }
  for(int i = 1; i < (7 - y); i++){
    result |= cartesian_to_int(x , y + i);
  }
  return result;
}



MoveData* get_move_data(Magics* MagicNumbers){
  //A 64 bit integer where all the edge parts of the board are 1
  uint64_t EDGES = 0xff818181818181ffULL;
  //Initialise Structure to return
  MoveData* ans = new MoveData;
  for (int x = 0; x < 8; x++){
      for (int y = 0; y < 8; y++){
          //Populate the hash tables for each possible position
          add_rook_moves(ans->r_moves,x,y,MagicNumbers);
          add_bishop_moves(ans->b_moves,x,y,MagicNumbers);
          //Generate the masks of all the possible move a piece could make in a position
          ans->r_mask[y*8 + x] = rook_mask_gen(x,y);
          ans->b_mask[y*8 + x] = bishop_attack(0,x,y) & ~EDGES;
          ans->kn_moves[y*8 + x] = knight_attack(x,y);
          ans->ki_moves[y*8 + x] = king_attack(x,y);
      }
  }
  return ans;
}



// int main(){
//   //A 64 bit integer where all the edge parts of the board are 1
//   Magics* magic_numbers = Get_Magics();
//   uint64_t EDGES = 0xff818181818181ffULL;
//   std::unordered_map<uint_fast16_t,uint64_t> rMoves[64];
//   std::unordered_map<uint_fast16_t,uint64_t> bMoves[64];
//   uint64_t rMask[64];
//   uint64_t bMask[64];

//   for (int i = 0; i < 64;i++){
//     std::cout << magic_numbers->r_magic[i] << std::endl;
//   }
//   //Generating all the things we need
//   for (int x = 0; x < 8; x++){
//       for (int y = 0; y < 8; y++){
//           add_rook_moves(rMoves,x,y,magic_numbers);
//           add_bishop_moves(bMoves,x,y,magic_numbers);
//           rMask[y*8 + x] = rook_attack(0,x,y) ^ EDGES;
//           bMask[y*8 + x] = bishop_attack(0,x,y) ^ EDGES;
//       }
//   }
//   int TotalSize = 0;
//   for(int i = 0; i < 64; i++){
//     std::cout << "The size is " << rMoves[i].size() << std::endl;
//     TotalSize += rMoves[i].size();
//   }
//   std::cout << TotalSize << std::endl;

//   TotalSize = 0;
//   for(int i = 0; i < 64; i++){
//     std::cout << "The size is " << bMoves[i].size() << std::endl;
//     TotalSize += bMoves[i].size();
//   }
//   std::cout << TotalSize << std::endl;
//   // int test = (10 * RMagic[0]) >> 52;
//   // std::cout << rMoves[0][test] << std::endl;
//   // std::cout << rMoves[0][0] << std::endl;
//   //std::cout << rMoves[0][test] << std::endl;
//   // for (int i = 0; i < 1000; i++){
//   //   std::cout << rMoves[0][i] << std::endl;
//   // }
//   //std::cout << rook_attack(0,1,0) << std::endl;
//   //std::cout << add_ones_rook(5,10,15,20,3,3) << std::endl;
//   //std::cout << bishop_attack(0,3,3) << std::endl;
// }

// int main(){
//   auto magicnumbers = Get_Magics();
//   get_move_data(magicnumbers);
//   std::cout << std::endl;
// }
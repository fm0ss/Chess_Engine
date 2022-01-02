#include<iostream>
#include<stdint.h>
#include<string>
#include<cctype>
#include<vector>
#include<unordered_map>
#include"magic.hpp"
#include"moves.hpp"
#include"move-database.hpp"
#include"DeBruijn.hpp"


//Important globals to be used throughout
std::unordered_map<uint_fast16_t,uint64_t>* rMoves;
std::unordered_map<uint_fast16_t,uint64_t>* bMoves;
uint64_t* rMask;
uint64_t* bMask;

//Welcome to fizzle engine
//I can't remember why they are named Pieces_B and I am too afraid to change it
typedef struct Pieces_B Pieces_B;
typedef struct Move Move;

//I make fast code, not good code.
// Struct to represent positions of each colour's pieces on a bitboard.
//The four different types of moves.
enum MoveType {Normal,Capture,EnPassant,Castling};


//Initialise Board to position with Fen notation
Board_State::Board_State(std::string Fen){
    //Change rank is bitshift by 8, Change file by bitshift of one
    //O(length of Fen)
    uint64_t position = 1;
    position <<= 63;
    for (auto &c : Fen){
        if (isdigit(c)){
            position >>= (c - '0');
        }
        //At this point in time I am pretending forward slashes don't exist and I can't see why it would be a problem
        else if (c != '/'){
            //I know this is disgusting but it is definitely not slow
            switch(c){
                case 'p':
                    Black.Pawns = Black.Pawns | position;break;
                case 'P':
                    White.Pawns = White.Pawns | position;break;
                case 'r':
                    Black.Rooks = Black.Rooks | position;break;
                case 'R':
                    White.Rooks = White.Rooks | position;break;
                case 'q':
                    Black.Queens = Black.Queens | position;break;
                case 'Q':
                    White.Queens = White.Queens | position;break;
                case 'b':
                    Black.Bishops = Black.Bishops | position;break;
                case 'B':
                    White.Bishops = White.Bishops | position;break;
                case 'n':
                    Black.Knights = Black.Knights | position;break;
                case 'N':
                    White.Knights = White.Knights | position;break;
                case 'k':
                    Black.King = Black.King | position;break;
                case 'K':
                    White.King = White.King | position;break;
            }
            position >>= 1;
        }
    }
}

//Just for debugging
void Board_State::Print_Colour(Pieces_B* colour){
    std::cout << colour->Bishops << " Bishops" << std::endl;
    std::cout << colour->Pawns << " Pawns" << std::endl;
    std::cout << colour->Rooks << " Rooks" << std::endl;
    std::cout << colour->Knights << " Knights" << std::endl;
    std::cout << colour->Queens << " Queens" << std::endl;
    std::cout << colour->King << " King" << std::endl;
}

//Also just for debugging
void Board_State::Print_Bitboard(){
    std::cout << "White" << std::endl;
    Print_Colour(&White);
    std::cout << "Black" << std::endl;
    Print_Colour(&Black);
}

//Starting to see the utility of bitboards
uint64_t Board_State::Get_All_Pieces(Pieces_B* colour){
    return colour->Bishops | colour->Pawns | colour->Rooks | colour->Knights | colour->Queens | colour->King;
}

uint64_t Board_State::Get_Board(){
    return Get_All_Pieces(&Black) | Get_All_Pieces(&White);
}

void Board_State::Add_Moves_Sub(int type,int from, uint64_t moves){
    int lsb;
    int pos = 0;
    while(moves != 0){
        lsb = DeBruijn(moves);
        pos += lsb;
        Add_Move(from,pos,type);
        moves >>= lsb + 1;
    }
}

void Board_State::Add_Moves(int from, uint64_t moves, Pieces_B* other){
    uint64_t attacks = moves & Get_All_Pieces(other);
    //Adding attack moves
    Add_Moves_Sub(1,from,attacks);
    //Ading normal moves
    Add_Moves_Sub(0,from,attacks ^ moves);
}

void Board_State::Gen_Sliding_Moves(Pieces_B* colour,Pieces_B* other,uint64_t piece,std::unordered_map<uint_fast16_t,uint64_t>* Moves,uint64_t* Magic, uint64_t* Bits,uint64_t* Mask){
    uint64_t our_pieces = Get_All_Pieces(colour);
    uint64_t all_blockers = Get_Board();
    uint64_t blockers;
    uint64_t moves;
    int pos = 0;
    int lsb;
    while(piece != 0){
        lsb = DeBruijn(piece);
        pos += lsb;
        blockers = Mask[pos] & all_blockers;
        moves = Moves[pos][(blockers * Magic[pos]) >> (64 - Bits[pos])];
        moves = moves ^ our_pieces;
        Add_Moves(pos,moves,other);
        piece >>= lsb + 1;
    }
}

void Board_State::Gen_Rook_Moves(Pieces_B* colour,Pieces_B* other){
    uint64_t our_pieces = Get_All_Pieces(colour);
    uint64_t rooks = colour->Rooks;
    uint64_t all_blockers = Get_Board();
    uint64_t blockers;
    uint64_t moves;
    int pos = 0;
    int lsb;
    while(rooks != 0){
        lsb = DeBruijn(rooks);
        pos += lsb;
        blockers = rMask[pos] & all_blockers;
        moves = rMoves[pos][(blockers * RMagic[pos]) >> (64 - RBits[pos])];
        moves = moves ^ our_pieces;
        Add_Moves(pos,moves,other);
        rooks >>= lsb + 1;
    }
}

void Board_State::Gen_Bishop_Moves(Pieces_B* colour,Pieces_B* other){
    uint64_t our_pieces = Get_All_Pieces(colour);
    uint64_t bishops = colour->Bishops;
    uint64_t all_blockers = Get_Board();
    uint64_t blockers;
    uint64_t moves;
    int pos = 0;
    int lsb;
    while(bishops != 0){
        lsb = DeBruijn(bishops);
        pos += lsb;
        blockers = bMask[pos] & all_blockers;
        moves = bMoves[pos][(blockers * BMagic[pos]) >> (64 - BBits[pos])];
        moves = moves ^ our_pieces;
        Add_Moves(pos,moves,other);
        bishops >>= lsb + 1;
    }
}

void Board_State::Gen_Queen_Moves(Pieces_B* colour,Pieces_B* other){
    Gen_Bishop_Moves(colour,other);
    Gen_Rook_Moves(colour,other);
}

void Board_State::Gen_White_Pawn_Moves(){
    uint64_t pawns = White.Pawns;
    uint64_t all_positions = Get_Board();
    uint64_t black_positions = Get_All_Pieces(&Black);
    uint64_t position;
    int pos = 0;
    int lsb;
    while(pawns != 0){
        lsb = DeBruijn(pawns);
        pos += lsb;
        position = 1ULL << pos;
        if ((all_positions & (position << 8)) == 0){
            Add_Move(pos,pos + 8,0);
        }
        //Two forward
        if ((all_positions & (position << 16)) == 0 && (pos / 8) == 1){
            Add_Move(pos,pos + 16,0);
        }
        //One forward and left/right
        if (black_positions & (position << 9) == 1){
            Add_Move(pos,pos + 9,1);
        }
        if (black_positions & (position << 7) == 1){
            Add_Move(pos,pos + 7,1);
        }
        pawns >>= lsb + 1;
    }
}

void Board_State::Gen_Black_Pawn_Moves(){
    uint64_t pawns = Black.Pawns;
    uint64_t all_positions = Get_Board();
    uint64_t white_positions = Get_All_Pieces(&White);
    uint64_t position;
    int pos = 0;
    int lsb;
    while(pawns != 0){
        lsb = DeBruijn(pawns);
        pos += lsb;
        position = 1ULL << pos;
        if ((all_positions & (position >> 8)) == 0){
            Add_Move(pos,pos - 8,0);
        }
        //Two forward
        if ((all_positions & (position >> 16)) == 0 && (pos / 8) == 1){
            Add_Move(pos,pos - 16,0);
        }
        //One forward and left/right
        if (white_positions & (position >> 9) == 1){
            Add_Move(pos,pos - 9,1);
        }
        if (white_positions & (position >> 7) == 1){
            Add_Move(pos,pos - 7,1);
        }
        pawns >>= lsb + 1;
    }
}

void Board_State::Gen_White_Moves(){
    Gen_Queen_Moves(&White,&Black);
    Gen_Rook_Moves(&White,&Black);
    Gen_Bishop_Moves(&White,&Black);
    Gen_White_Pawn_Moves();
}

void Board_State::Gen_Black_Moves(){
    Gen_Queen_Moves(&Black,&White);
    Gen_Rook_Moves(&Black,&White);
    Gen_Bishop_Moves(&Black,&White);
    Gen_Black_Pawn_Moves();
}



void Board_State::Add_Move(uint_fast16_t origin,uint_fast16_t destination,uint_fast16_t type){
    //Resize in chunks of 32 as of now.
    //Average chess position is about 30 moves
    if (MoveIndex % 32 == 0){
        Moves.resize(MoveIndex + 32);
    }
    Moves[MoveIndex] = type + (destination << 4) + (origin << 10);
    MoveIndex++;
}


int main(){
    //Board_State test("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR");
    MoveData* ans = get_move_data();
    rMask = ans->rMask;bMask = ans->bMask;
    rMoves = ans->rMoves;bMoves = ans->bMoves;
    Board_State test;
    std::cout << test.Black.Bishops << std::endl;
    // for(int i = 0; i < test.MoveIndex;i++){
    //     std::cout << test.Moves[i] << std::endl;
    // }0x022fdd63cc95386d
    // std::cout << "done" << std::endl;
    // std::cout << test.MoveIndex << std::endl;
    //MY_GLOBALS_H::RMagic
    std::cout << RMagic[0] << std::endl;
}
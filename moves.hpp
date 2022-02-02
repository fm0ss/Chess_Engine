#include<iostream>
#include<stdint.h>
#include<string>
#include<cctype>
#include<vector>
#include<unordered_map>
#include"move-database.hpp"
#include"DeBruijn.hpp"


struct Pieces_B{
    uint64_t pawns = 0;
    uint64_t rooks = 0;
    uint64_t knights = 0;
    uint64_t king = 0;
    uint64_t bishops = 0;
    uint64_t queens = 0;
    bool queen_side = true;
    bool king_side = true;
    std::vector<uint64_t*> all_bitboards;
    void generate_bitboards();
};

class Board_State{
    Magics* magic_numbers;
    MoveData* move_database;
    Pieces_B black,white;
    public:
        std::vector<uint_fast16_t> Moves = std::vector<uint_fast16_t>(32,0);
        int move_index = 0;
        Board_State(std::string);
        //Delegating constructor for default
        Board_State() : Board_State("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"){}
        Board_State(std::string,MoveData*,Magics*);
        void print_bitboard();
        static void print_colour(Pieces_B*);
        static uint64_t get_all_pieces(Pieces_B*);
        uint64_t get_board();
        bool king_attacked();
        void make_move(uint_fast16_t);
        void get_all_moves_white();
        void gen_rook_moves(Pieces_B*,Pieces_B*);
        void gen_bishop_moves(Pieces_B*,Pieces_B*);
        void gen_queen_moves(Pieces_B*,Pieces_B*);
        void gen_k_moves(Pieces_B*,Pieces_B*,uint64_t,uint64_t*);
        void gen_white_pawn_moves();
        void gen_black_pawn_moves();
        void gen_white_moves();
        void gen_black_moves();
        void gen_sliding_moves(Pieces_B* ,Pieces_B* ,uint64_t ,std::unordered_map<uint_fast16_t,uint64_t>* ,const uint64_t* , const int* ,uint64_t*);
        void add_move(uint_fast16_t,uint_fast16_t,uint_fast16_t);
        void add_moves(int , uint64_t ,Pieces_B* );
        void add_moves_sub(int ,int , uint64_t );
};
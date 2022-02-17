#include<iostream>
#include<stdint.h>
#include<string>
#include<cctype>
#include<vector>
#include<unordered_map>
#include"string.h"
#include"move-database.hpp"
#include"DeBruijn.hpp"



//I decided on using a struct here because this is really just data with useful functions with it
//Not that there is much meaningfull difference between a struct and a class
struct Pieces_B{
    uint64_t pawns = 0;
    uint64_t rooks = 0;
    uint64_t knights = 0;
    uint64_t king = 0;
    uint64_t bishops = 0;
    uint64_t queens = 0;
    uint64_t pawns_copy = 0;
    uint64_t rooks_copy = 0;
    uint64_t knights_copy = 0;
    uint64_t king_copy = 0;
    uint64_t bishops_copy = 0;
    uint64_t queens_copy = 0;
    uint64_t all_pieces;
    bool queen_side = true;
    bool king_side = true;
    std::vector<uint64_t*> all_bitboards;
    void generate_bitboards();
    uint64_t set_all_pieces();
    void promote_piece(uint64_t, int);
    void update_copys();
    void set_to_copys();
};

class Board_State{
    Magics* magic_numbers;
    MoveData* move_database;
    public:
        Pieces_B black,white;
        std::vector<uint_fast16_t> Moves = std::vector<uint_fast16_t>(32,0);
        uint_fast16_t prev_move = 0;
        int move_index = 0;
        bool white_to_move = true;
        Board_State(std::string);
        //Delegating constructor for default
        Board_State() : Board_State("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"){}
        Board_State(std::string,MoveData*,Magics*);
        Board_State(Board_State*,uint_fast16_t);
        void set_castling_info();
        void pretty_print_bitboard();
        void print_bitboard();
        static void print_colour(Pieces_B*);
        static uint64_t get_all_pieces(Pieces_B*);
        uint64_t get_board();
        bool king_attacked();
        bool black_king_attacked();
        bool white_king_attacked();
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
        void gen_white_castling_moves();
        void gen_black_castling_moves();
        void gen_sliding_moves(Pieces_B* ,Pieces_B* ,uint64_t ,std::unordered_map<uint_fast16_t,uint64_t>* ,const uint64_t* , const int* ,uint64_t*);
        void add_move(uint_fast16_t,uint_fast16_t,uint_fast16_t);
        void add_moves(int , uint64_t ,Pieces_B* );
        void add_moves_sub(int ,int , uint64_t );
        bool validate_move(uint_fast16_t);
        void unmake_move();
        void add_white_pawn_move(uint_fast16_t,uint_fast16_t,uint_fast16_t);
        void add_black_pawn_move(uint_fast16_t,uint_fast16_t,uint_fast16_t);
        void get_board_for_eval(std::vector<float>&);
        void add_piece_for_eval(std::vector<float>&,uint64_t,int start);
        void gen_moves();
        static uint64_t flip_integer(uint64_t);
};
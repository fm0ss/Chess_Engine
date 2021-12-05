#include<stdint.h>
#include<string>
#include<vector>

struct Pieces_B{
    uint64_t Pawns = 0;
    uint64_t Rooks = 0;
    uint64_t Knights = 0;
    uint64_t King = 0;
    uint64_t Bishops = 0;
    uint64_t Queens = 0;
    bool Queen_Side = true;
    bool King_Side = true;
};

class Board_State{
    public:
        //Make private later will break debugging methods
        Pieces_B Black,White;
        std::vector<uint_fast16_t> Moves = std::vector<uint_fast16_t>(32,0);
        int MoveIndex = 0;
        Board_State(std::string);
        //Delegating constructor for default
        Board_State() : Board_State("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"){}
        void Print_Bitboard();
        static void Print_Colour(Pieces_B*);
        static uint64_t Get_All_Pieces(Pieces_B*);
        uint64_t Get_Board();
        void Get_All_Moves();
        void Add_Move(uint_fast16_t,uint_fast16_t,uint_fast16_t);
};
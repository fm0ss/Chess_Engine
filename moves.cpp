#include<iostream>
#include<stdint.h>
#include<string>
#include<cctype>
#include<vector>

//Welcome to fizzle engine
//I can't remember why they are named Pieces_B and I am too afraid to change it
typedef struct Pieces_B Pieces_B;
typedef struct Move Move;

//I make fast code, not good code.
// Struct to represent positions of each colour's pieces on a bitboard.
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

//The four different types of moves.
enum MoveType {Normal,Capture,EnPassant,Castling};


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
        void Print_Colour(Pieces_B);
        uint64_t Get_All_Pieces(Pieces_B);
        uint64_t Get_Board();
        Move* Get_White_Pawn_Moves();
        void Get_All_Moves();
        void Add_Move(uint_fast16_t,uint_fast16_t,uint_fast16_t);
};

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
void Board_State::Print_Colour(Pieces_B colour){
    std::cout << colour.Bishops << " Bishops" << std::endl;
    std::cout << colour.Pawns << " Pawns" << std::endl;
    std::cout << colour.Rooks << " Rooks" << std::endl;
    std::cout << colour.Knights << " Knights" << std::endl;
    std::cout << colour.Queens << " Queens" << std::endl;
    std::cout << colour.King << " King" << std::endl;
}

//Also just for debugging
void Board_State::Print_Bitboard(){
    std::cout << "White" << std::endl;
    Print_Colour(White);
    std::cout << "Black" << std::endl;
    Print_Colour(Black);
}

//Starting to see the utility of bitboards
uint64_t Board_State::Get_All_Pieces(Pieces_B colour){
    return colour.Bishops | colour.Pawns | colour.Rooks | colour.Knights | colour.Queens | colour.King;
}

uint64_t Board_State::Get_Board(){
    return Get_All_Pieces(Black) | Get_All_Pieces(White);
}

void Board_State::Get_All_Moves(){
    int64_t white_positions = Get_All_Pieces(White);
    int64_t black_positions = Get_All_Pieces(Black);
    int64_t all_positions = Get_Board();
    uint_fast16_t count = 0;
    int64_t position = 1;
    //First 6 bits are origin
    //Second 6 are the destination
    //Last 4 are for the movetype
    uint_fast16_t move;
    while(white_positions != 0){
        if ((white_positions & 1) == 1){
            //There is a white piece here.
            if ((White.Pawns >> count) & 1 == 1){
                if ((all_positions & (position << 8)) == 0){
                    Add_Move(count,count + 8,0);
                }
                if ((all_positions & (position << 16)) == 0 && (count / 8) == 1){
                    Add_Move(count,count + 16,0);
                }
                if (black_positions & (position << 9) == 1){
                    Add_Move(count,count + 9,1);
                }
                if (black_positions & (position << 7) == 1){
                    Add_Move(count,count + 7,1);
                }
            }
        }
        //Bitshifts everywhere yet I still can't remember which is which
        white_positions >>= 1;
        position <<= 1;
        count++;
    }
}

void Board_State::Add_Move(uint_fast16_t origin,uint_fast16_t destination,uint_fast16_t type){
    Moves[MoveIndex] = type + (destination << 4) + (origin << 10);
    MoveIndex++;
}


int main(){
    //Board_State test("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR");
    Board_State test;
    test.Get_All_Moves();
    for(int i = 0; i < test.MoveIndex;i++){
        std::cout << test.Moves[i] << std::endl;
    }
    std::cout << "done" << std::endl;
    std::cout << test.MoveIndex;
}
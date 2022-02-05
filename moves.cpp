#include"moves.hpp"

//todo
//1. Make things pass by reference as appropriate
//2. Go from psuedo legal to legal

//Naming convention use lower case for all locals/args and Caps for everything else
typedef struct Pieces_B Pieces_B;
typedef struct Move Move;

// Struct to represent positions of each colour's pieces on a bitboard.
//The four different types of moves.
enum MoveType {Normal,Capture,EnPassant,Castling};


void Pieces_B::generate_bitboards(){
    all_bitboards.push_back(&rooks);
    all_bitboards.push_back(&bishops);
    all_bitboards.push_back(&queens);
    all_bitboards.push_back(&king);
    all_bitboards.push_back(&pawns);
    all_bitboards.push_back(&knights);
}


//Initialise Board to position with Fen notation
Board_State::Board_State(std::string fen,MoveData* move_database, Magics* magic_numbers){
    //Change rank is bitshift by 8, Change file by bitshift of one
    //O(length of Fen)
    white.generate_bitboards();
    black.generate_bitboards();
    this->move_database = move_database;
    this->magic_numbers = magic_numbers;
    int pos = 56;
    uint64_t position;
    //std::cout << position;
    for (auto &c : fen){
        position = 1ULL << pos;
        if (isdigit(c)){
            pos += (c - '0');
        }
        else if (c != '/'){
            switch(c){
                case 'p':
                    black.pawns = black.pawns | position;break;
                case 'P':
                    white.pawns = white.pawns | position;break;
                case 'r':
                    black.rooks = black.rooks | position;break;
                case 'R':
                    white.rooks = white.rooks | position;break;
                case 'q':
                    black.queens = black.queens | position;break;
                case 'Q':
                    white.queens = white.queens | position;break;
                case 'b':
                    black.bishops = black.bishops | position;break;
                case 'B':
                    white.bishops = white.bishops | position;break;
                case 'n':
                    black.knights = black.knights | position;break;
                case 'N':
                    white.knights = white.knights | position;break;
                case 'k':
                    black.king = black.king | position;break;
                case 'K':
                    white.king = white.king | position;break;
            }
            pos++;
        }
        else{
            pos -= 16;
        }
    }
}


//Just for debugging
void Board_State::print_colour(Pieces_B* colour){
    std::cout << colour->bishops << " bishops" << std::endl;
    std::cout << colour->pawns << " pawns" << std::endl;
    std::cout << colour->rooks << " rooks" << std::endl;
    std::cout << colour->knights << " knights" << std::endl;
    std::cout << colour->queens << " queens" << std::endl;
    std::cout << colour->king << " king" << std::endl;
}

//Also just for debugging
void Board_State::print_bitboard(){
    std::cout << "white" << std::endl;
    print_colour(&white);
    std::cout << "black" << std::endl;
    print_colour(&black);
}

//Starting to see the utility of bitboards
uint64_t Board_State::get_all_pieces(Pieces_B* colour){
    return colour->bishops | colour->pawns | colour->rooks | colour->knights | colour->queens | colour->king;
}

uint64_t Board_State::get_board(){
    return get_all_pieces(&black) | get_all_pieces(&white);
}

void Board_State::add_moves_sub(int type,int from, uint64_t moves){
    int lsb;
    int pos = -1;
    while(moves != 0){
        lsb = debruijn(moves);
        pos += lsb + 1;
        add_move(from,pos,type);
        moves >>= lsb + 1;
    }
}

void Board_State::add_moves(int from, uint64_t moves, Pieces_B* other){
    uint64_t attacks = moves & get_all_pieces(other);
    //Adding attack moves
    add_moves_sub(1,from,attacks);
    //Adding normal moves
    add_moves_sub(0,from,attacks ^ moves);
}

void Board_State::gen_sliding_moves(Pieces_B* colour,Pieces_B* other,uint64_t piece,std::unordered_map<uint_fast16_t,uint64_t>* Moves,const uint64_t* Magic,const int* Bits,uint64_t* Mask){
    uint64_t our_pieces = get_all_pieces(colour);
    uint64_t all_blockers = get_board();
    uint64_t blockers;
    uint64_t moves;
    int pos = -1;
    int lsb;
    while(piece != 0){
        lsb = debruijn(piece);
        pos += lsb + 1;
        blockers = Mask[pos] & all_blockers;
        moves = Moves[pos][(blockers * Magic[pos]) >> (64 - Bits[pos])];
        //So we don't take our own pieces
        moves = (moves & ~our_pieces);
        add_moves(pos,moves,other);
        piece >>= lsb + 1;
    }
}

void Board_State::gen_rook_moves(Pieces_B* colour,Pieces_B* other){
    gen_sliding_moves(colour,other,colour->rooks,move_database->r_moves,magic_numbers->r_magic,magic_numbers->r_bits,move_database->r_mask);
}

void Board_State::gen_bishop_moves(Pieces_B* colour,Pieces_B* other){
    gen_sliding_moves(colour,other,colour->bishops,move_database->b_moves,magic_numbers->b_magic,magic_numbers->b_bits,move_database->b_mask);
}

void Board_State::gen_queen_moves(Pieces_B* colour,Pieces_B* other){
    gen_sliding_moves(colour,other,colour->queens,move_database->b_moves,magic_numbers->b_magic,magic_numbers->b_bits,move_database->b_mask);
    gen_sliding_moves(colour,other,colour->queens,move_database->r_moves,magic_numbers->r_magic,magic_numbers->r_bits,move_database->r_mask);
}

void Board_State::gen_k_moves(Pieces_B* colour,Pieces_B* other,uint64_t piece,uint64_t* move_lookup){
    uint64_t moves;
    uint64_t our_pieces = get_all_pieces(colour);
    int pos = -1;
    int lsb;
    while(piece != 0){
        lsb = debruijn(piece);
        pos += lsb + 1;
        moves = move_lookup[pos];
        //So we don't take our own pieces
        moves = (moves & ~our_pieces);
        add_moves(pos,moves,other);
        piece >>= lsb + 1;
    }
}

void Board_State::gen_white_pawn_moves(){
    uint64_t pawns = white.pawns;
    uint64_t all_positions = get_board();
    uint64_t black_positions = get_all_pieces(&black);
    uint64_t position;
    //Init to -1 since the first time we add one more than we'd want to
    int pos = -1;
    int lsb;
    while(pawns != 0){
        lsb = debruijn(pawns);
        pos += lsb + 1;
        position = 1ULL << pos;
        if ((all_positions & (position << 8)) == 0){
            add_move(pos,pos + 8,0);
        }
        //Two forward
        if ((all_positions & (position << 16)) == 0 && (pos / 8) == 1){
            add_move(pos,pos + 16,0);
        }
        //One forward and left/right
        if (black_positions & (position << 9) == 1){
            add_move(pos,pos + 9,1);
        }
        if (black_positions & (position << 7) == 1){
            add_move(pos,pos + 7,1);
        }
        pawns >>= lsb + 1;

    }
}

void Board_State::gen_black_pawn_moves(){
    uint64_t pawns = black.pawns;
    uint64_t all_positions = get_board();
    uint64_t white_positions = get_all_pieces(&white);
    uint64_t position;
    int pos = -1;
    int lsb;
    while(pawns != 0){
        lsb = debruijn(pawns);
        pos += lsb + 1;
        position = 1ULL << pos;
        if ((all_positions & (position >> 8)) == 0){
            add_move(pos,pos - 8,0);
        }
        //Two forward
        if ((all_positions & (position >> 16)) == 0 && (pos / 8) == 1){
            add_move(pos,pos - 16,0);
        }
        //One forward and left/right
        if (white_positions & (position >> 9) == 1){
            add_move(pos,pos - 9,1);
        }
        if (white_positions & (position >> 7) == 1){
            add_move(pos,pos - 7,1);
        }
        pawns >>= lsb + 1;
    }
}

void Board_State::gen_white_moves(){
    gen_queen_moves(&white,&black);
    gen_rook_moves(&white,&black);
    gen_bishop_moves(&white,&black);
    gen_k_moves(&white,&black,white.knights,move_database->kn_moves);
    gen_k_moves(&white,&black,white.king,move_database->ki_moves);
    gen_white_pawn_moves();
}

void Board_State::gen_black_moves(){
    gen_queen_moves(&black,&white);
    gen_rook_moves(&black,&white);
    gen_bishop_moves(&black,&white);
    gen_k_moves(&black,&white,black.knights,move_database->kn_moves);
    gen_k_moves(&black,&white,black.king,move_database->ki_moves);
    gen_black_pawn_moves();
}



void Board_State::add_move(uint_fast16_t origin,uint_fast16_t destination,uint_fast16_t type){
    //Resize in chunks of 32 as of now.
    //Average chess position is about 30 moves
    //std::cout << origin << " " << destination << std::endl;
    if (move_index % 32 == 0){
        Moves.resize(move_index + 32);
    }
    std::cout << "from " << origin << " to " << destination << std::endl;
    Moves[move_index] = type + (destination << 4) + (origin << 10);
    move_index++;
}


bool Board_State::king_attacked(){
    //babers why
    int pos = debruijn(white.king);
    uint64_t check = 0;
    uint64_t blockers, moves;

    //Check knight moves
    check |= move_database->kn_moves[pos] & black.knights;

    //Check Pawns
    check |= black.pawns & ((white.king >> 9) + (white.king >> 7));

    //The way we check if the king is attacked by bishops, rooks and queens
    //is by pretending that the king is each of these pieces sequentially,
    //then seeing if we can attack any of the other pieces from the king.

    //Check Rooks/Queens
    blockers = (get_board() ^ (black.rooks | black.queens)) & move_database->r_mask[pos];
    moves = move_database->r_moves[pos][(blockers * magic_numbers->r_magic[pos]) >> magic_numbers->r_magic[pos]];
    check |= moves & (black.rooks | black.queens);

    //Check Bishops/Queens
    blockers = (get_board() ^ (black.bishops | black.queens)) & move_database->b_mask[pos];
    moves = move_database->b_moves[pos][(blockers * magic_numbers->b_magic[pos]) >> magic_numbers->b_magic[pos]];
    check |= moves & (black.bishops | black.queens);

    //If check is 0 the king is not attacked
    return !check;
}

void Board_State::make_move(uint_fast16_t move){
    uint64_t origin = 1 << (move >> 10);
    uint64_t destination = 1 << ((move & 0xfc00) >> 4);
    int type = (move & 0xfff0);
    if (type == 1 | type == 0){
        for (auto i : white.all_bitboards){
            if (*i & origin != 0){
                *i = *i | destination;
                *i = *i & !origin;
            }
            else{
                *i = *i & !destination;
            }
        //*i = *i | (destination * (int)(bool)(*i & origin));
        //This is a branchless solution test if it is better
        }
        for (auto i : black.all_bitboards){
            if (*i & origin != 0){
                *i = *i | destination;
                *i = *i & !origin;
            }
            else{
                *i = *i & !destination;
            }
        }
    }
}



int main(){
    //Board_State test("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR");
    Magics* magic_numbers = Get_Magics();
    MoveData* move_database = get_move_data(magic_numbers);
    Board_State test("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",move_database,magic_numbers);

    test.gen_white_moves();

    std::cout << "Moves generated" << std::endl;

    std::cout << test.Moves.size() << std::endl;
    // for(int i = 0; i < test.MoveIndex;i++){
    //     std::cout << test.Moves[i] << std::endl;
    // }0x022fdd63cc95386d
    // std::cout << "done" << std::endl;
    // std::cout << test.MoveIndex << std::endl;
    //MY_GLOBALS_H::RMagic
}
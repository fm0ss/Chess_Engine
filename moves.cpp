#include"moves.hpp"

typedef struct Pieces_B Pieces_B;
typedef struct Move Move;

//Struct to represent positions of each colour's pieces on a bitboard.
//Represents all the moves to make
//Maintaining the subset from PROMOTE_KNIGHT to PROMOTE_QUEEN is important since they are for loop bounds 
enum MoveType {NORMAL,CAPTURE,ENPASSANT,CASTLING,PROMOTE_KNIGHT,PROMOTE_BISHOP,PROMOTE_ROOK,PROMOTE_QUEEN};

//I needed a list of all the bitboard memory locations to make the move making function more simple
void Pieces_B::generate_bitboards(){
    all_bitboards.push_back(&rooks);
    all_bitboards.push_back(&bishops);
    all_bitboards.push_back(&queens);
    all_bitboards.push_back(&king);
    all_bitboards.push_back(&pawns);
    all_bitboards.push_back(&knights);
}

//Originally I was calculating this each time I needed, but it is used so much it is better to calculate it each time
uint64_t Pieces_B::set_all_pieces(){
    return all_pieces = bishops | pawns | rooks | knights | queens | king;
}

//Promotes to the correct piece for each different type
void Pieces_B::promote_piece(uint64_t destination, int type){
    switch(type){
        case PROMOTE_KNIGHT:
            knights |= destination;break;
        case PROMOTE_BISHOP:
            bishops |= destination;break;
        case PROMOTE_ROOK:
            rooks |= destination;break;
        case PROMOTE_QUEEN:
            queens |= destination;break;
    }
}

//Makes a copy of each bitboard so that when validating moves we can go back after
//A downside of psuedo legal move generation
void Pieces_B::update_copys(){
    pawns_copy = pawns;
    rooks_copy = rooks;
    queens_copy = queens;
    king_copy = king;
    knights_copy = queens;
    bishops_copy = bishops;
}

//Returns to the state before a move was made
//Will be called externally from the tree search
void Pieces_B::set_to_copys(){
    pawns = pawns_copy;
    rooks = rooks_copy;
    queens = queens_copy;
    king = king_copy;
    knights = knights_copy;
    bishops = bishops_copy;
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
    white.set_all_pieces();
    black.set_all_pieces();
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

//Deprecated - remove later
uint64_t Board_State::get_all_pieces(Pieces_B* colour){
    return colour->bishops | colour->pawns | colour->rooks | colour->knights | colour->queens | colour->king;
}

//Returns a bitboard of all piece positions
uint64_t Board_State::get_board(){
    return white.all_pieces | black.all_pieces;
}

//Given a list of moves in the form of a 64 bit integer it adds all the moves to the move list
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


//Splits captures and normal moves apart
void Board_State::add_moves(int from, uint64_t moves, Pieces_B* other){
    uint64_t attacks = moves & other->all_pieces;
    //Adding attack moves
    add_moves_sub(CAPTURE,from,attacks);
    //Adding normal moves
    add_moves_sub(NORMAL,from,attacks ^ moves);
}

//For sliding pieces(rooks, queens and bishops) this is where the move generation is done
//it loops through the 1s of the 64 bit integer, then uses perfect hashing techniques to lookup
//what moves can be made
void Board_State::gen_sliding_moves(Pieces_B* colour,Pieces_B* other,uint64_t piece,std::unordered_map<uint_fast16_t,uint64_t>* Moves,const uint64_t* Magic,const int* Bits,uint64_t* Mask){
    uint64_t our_pieces = colour->all_pieces;
    uint64_t all_blockers = get_board();
    uint64_t blockers;
    uint64_t moves;
    int pos = -1;
    int lsb;
    while(piece != 0){
        lsb = debruijn(piece);
        pos += lsb + 1;
        //Calculate moves
        blockers = Mask[pos] & all_blockers;
        moves = Moves[pos][(blockers * Magic[pos]) >> (64 - Bits[pos])];
        //So we don't take our own pieces
        moves = (moves & ~our_pieces);
        add_moves(pos,moves,other);
        piece >>= lsb + 1;
    }
}

//The following functions are just how each of the sliding pieces' generation use the above method

void Board_State::gen_rook_moves(Pieces_B* colour,Pieces_B* other){
    gen_sliding_moves(colour,other,colour->rooks,move_database->r_moves,magic_numbers->r_magic,magic_numbers->r_bits,move_database->r_mask);
}

void Board_State::gen_bishop_moves(Pieces_B* colour,Pieces_B* other){
    gen_sliding_moves(colour,other,colour->bishops,move_database->b_moves,magic_numbers->b_magic,magic_numbers->b_bits,move_database->b_mask);
}

//A queen is basically a rook and a bishop, and can be treated as such
void Board_State::gen_queen_moves(Pieces_B* colour,Pieces_B* other){
    gen_sliding_moves(colour,other,colour->queens,move_database->b_moves,magic_numbers->b_magic,magic_numbers->b_bits,move_database->b_mask);
    gen_sliding_moves(colour,other,colour->queens,move_database->r_moves,magic_numbers->r_magic,magic_numbers->r_bits,move_database->r_mask);
}

//This is for kings and knights, for each position we lookup all the possible moves that could be made by that piece in that position
void Board_State::gen_k_moves(Pieces_B* colour,Pieces_B* other,uint64_t piece,uint64_t* move_lookup){
    uint64_t moves;
    uint64_t our_pieces = colour->all_pieces;
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
    uint64_t black_positions = black.all_pieces;
    uint64_t position;
    //Init to -1 since the first time we add one more than we'd want to
    int pos = -1;
    int lsb;
    while(pawns != 0){
        lsb = debruijn(pawns);
        pos += lsb + 1;
        position = 1ULL << pos;
        if ((all_positions & (position << 8)) == 0){
            add_white_pawn_move(pos,pos + 8,NORMAL);
        }
        //Two forward
        if ((all_positions & (position << 16)) == 0 && (pos / 8) == 1){
            add_move(pos,pos + 16,NORMAL);
        }
        //One forward and left/right
        if (black_positions & (position << 9) == 1){
            add_white_pawn_move(pos,pos + 9,CAPTURE);
        }
        if (black_positions & (position << 7) == 1){
            add_white_pawn_move(pos,pos + 7,CAPTURE);
        }
        pawns >>= lsb + 1;

    }
    //En Passant Check
    uint64_t origin = (1ULL << (prev_move >> 10));
    uint64_t destination = (1ULL << ((prev_move & 0x3f0) >> 4));

    //If the previous move was a double pawn push and behind the pawn is clear
    if ((black.pawns & destination) != 0 & (origin / destination) == (1ULL << 16) & (((destination << 8) & get_board()) == 0)){
        if ((white.pawns & (destination << 1)) != 0){
            add_move(debruijn(destination << 1),debruijn(destination << 8),2);
        }
        if ((white.pawns & (destination >> 1)) != 0){
            add_move(debruijn(destination >> 1),debruijn(destination << 8),2);
        }
    }
}

void Board_State::gen_black_pawn_moves(){
    uint64_t pawns = black.pawns;
    uint64_t all_positions = get_board();
    uint64_t white_positions = white.all_pieces;
    uint64_t position;
    int pos = -1;
    int lsb;
    while(pawns != 0){
        lsb = debruijn(pawns);
        pos += lsb + 1;
        position = 1ULL << pos;
        if ((all_positions & (position >> 8)) == 0){
            add_black_pawn_move(pos,pos - 8,NORMAL);
        }
        //Two forward
        if ((all_positions & (position >> 16)) == 0 && (pos / 8) == 6){
            add_move(pos,pos - 16,NORMAL);
        }
        //One forward and left/right
        if (white_positions & (position >> 9) == 1){
            add_black_pawn_move(pos,pos - 9,CAPTURE);
        }
        if (white_positions & (position >> 7) == 1){
            add_black_pawn_move(pos,pos - 7,CAPTURE);
        }
        pawns >>= lsb + 1;
    }
    //En Passant Check
    uint64_t origin = (1ULL << (prev_move >> 10));
    uint64_t destination = (1ULL << ((prev_move & 0x3f0) >> 4));
    
    //If the previous move was a double pawn push and behind the pawn is clear
    if ((white.pawns & destination) != 0 & (destination / origin) == (1ULL << 16) & (((destination >> 8) & get_board()) == 0)){
        if ((black.pawns & (destination << 1)) != 0){
            add_move(debruijn(destination << 1),debruijn(destination >> 8),2);
        }
        if ((white.pawns & (destination >> 1)) != 0){
            add_move(debruijn(destination >> 1),debruijn(destination >> 8),2);
        }
    }
}

void Board_State::gen_white_castling_moves(){
    //Check if we can castle and that the space is clear
    if (white.queen_side == true & (0x7 & get_board()) == 0){
        add_move(4,2,3);
    }
    if (white.king_side == true & (0x60 & get_board()) == 0){
        add_move(4,6,3);
    }
}

void Board_State::gen_black_castling_moves(){
    //Check if we can castle and that the space is clear
    if (black.queen_side == true & (0x700000000000000 & get_board()) == 0){
        add_move(60,58,3);
    }
    if (black.king_side == true & (0x6000000000000000 & get_board()) == 0){
        add_move(60,62,3);
    }
}


void Board_State::gen_white_moves(){
    white.update_copys();
    black.update_copys();
    Moves.clear();
    move_index = 0;
    gen_queen_moves(&white,&black);
    gen_rook_moves(&white,&black);
    gen_bishop_moves(&white,&black);
    gen_k_moves(&white,&black,white.knights,move_database->kn_moves);
    gen_k_moves(&white,&black,white.king,move_database->ki_moves);
    gen_white_pawn_moves();
}

void Board_State::gen_black_moves(){
    white.update_copys();
    black.update_copys();
    Moves.clear();
    move_index = 0;
    gen_queen_moves(&black,&white);
    gen_rook_moves(&black,&white);
    gen_bishop_moves(&black,&white);
    gen_k_moves(&black,&white,black.knights,move_database->kn_moves);
    gen_k_moves(&black,&white,black.king,move_database->ki_moves);
    gen_black_pawn_moves();
}

//Seperate functions for adding pawn moves are needed in order to handle promotions
void Board_State::add_white_pawn_move(uint_fast16_t origin,uint_fast16_t destination,uint_fast16_t type){
    //If it is a promotion
    if (destination / 8 == 7){
        for (int i = PROMOTE_KNIGHT; i <= PROMOTE_QUEEN;i++){
            add_move(origin,destination,i);
        }
    }
    else{
        add_move(origin,destination,type);
    }
}

void Board_State::add_black_pawn_move(uint_fast16_t origin,uint_fast16_t destination,uint_fast16_t type){
    //If it is a promotion
    if (destination / 8 == 0){
        for (int i = PROMOTE_KNIGHT; i <= PROMOTE_QUEEN;i++){
            add_move(origin,destination,i);
        }
    }
    else{
        add_move(origin,destination,type);
    }
}

void Board_State::add_move(uint_fast16_t origin,uint_fast16_t destination,uint_fast16_t type){
    //Resize in chunks of 32.
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
    return check;
}

void Board_State::make_move(uint_fast16_t move){
    prev_move = move;
    uint64_t origin = (1ULL << (move >> 10));
    uint64_t destination = (1ULL << ((move & 0x3f0) >> 4));
    int type = (move & 0xf);
    if (type <= 3){
        //Find which bitboard needs the move made and make it
        for (auto i : white.all_bitboards){
            if ((*i & origin) != 0){
                *i = *i | destination;
                *i = *i & ~origin;
            }
            else{
                *i = *i & ~destination;
            }
        //*i = *i | (destination * (int)(bool)(*i & origin));
        //This is a branchless solution test if it is better
        }
        for (auto i : black.all_bitboards){
            if ((*i & origin) != 0){
                *i = *i | destination;
                *i = *i & ~origin;
            }
            else{
                *i = *i & ~destination;
            }
        }
        //En passant
        if (type == 2){
            //If it was white's move
            if (white_to_move){
                black.pawns = black.pawns & ~(destination >> 8); 
            }
            //Black's move
            else{
                white.pawns = white.pawns & ~(destination << 8);
            }
        }
        //Castling
        //Only the rooks need to be moved, since the above code moved the king
        else if (type == 3){
            if(white_to_move){
                //Queenside
                if (destination == 0x4){
                    white.rooks = white.rooks ^ 0x9;
                }
                //Kingside
                else{
                    white.rooks = white.rooks ^ 0xa0;
                }
            }
            else{
                //Queenside
                if (destination == 0x400000000000000){
                    white.rooks = white.rooks ^ 0x900000000000000;
                }
                //Kingside
                else{
                    white.rooks = white.rooks ^ 0xa000000000000000;
                }
            }
        } 
    }
    //Promoting pawns
    else{
        //This is faster than checking which to change
        //We remove the origin location from both, one of them contains it.
        white.pawns = white.pawns & ~origin;
        black.pawns = black.pawns & ~origin;

        //If white
        if (white_to_move){
            white.promote_piece(destination,type);
        }
        else{
            black.promote_piece(destination,type);
        }
    }
    white_to_move = !white_to_move;
    white.set_all_pieces();
    black.set_all_pieces();
    //Castling information is done upon initialisation of a Board_State object
    //The reason it's not done here is because whenever we select a move
    //We go to that new board, so it makes sense for it to be done externally
    //A function will look at the parent nodes castling information then change it if the move affected castling
}

bool Board_State::validate_move(uint_fast16_t move){
    //Make a move then check if the king is attack,
    //and return whether or not it is valid
    //If it is not valid the move will be undone
    make_move(move);
    return king_attacked();
}

//In order to undo the above function or calls to make_move() in general
void Board_State::unmake_move(){
    white.set_to_copys();
    black.set_to_copys();
}


int main(){
    //Board_State test("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR");
    Magics* magic_numbers = Get_Magics();
    MoveData* move_database = get_move_data(magic_numbers);
    Board_State test("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",move_database,magic_numbers);

    std::cout << test.get_board() << std::endl;

    test.gen_white_moves();

    std::cout << test.Moves[5] << std::endl;

    test.make_move(test.Moves[5]);

    test.gen_white_moves();

    std::cout << test.Moves[20] << std::endl;

    test.make_move(test.Moves[20]);

    test.gen_black_moves();

    std::cout << test.Moves[6] << std::endl;

    std::cout << test.get_board() << std::endl;

    test.make_move(test.Moves[6]);

    std::cout << test.get_board() << std::endl;

    test.gen_white_moves();

    std::cout << "Moves generated" << std::endl;

    std::cout << test.Moves.size() << std::endl;
    // for(int i = 0; i < test.MoveIndex;i++){
    //     std::cout << test.Moves[i] << std::endl;
    // }0x022fdd63cc95386d
    // std::cout << "done" << std::endl;
    // std::cout << test.MoveIndex << std::endl;
    //MY_GLOBALS_H::100001RMagic
}
#include"moves.hpp"

typedef struct Pieces_B Pieces_B;
typedef struct Move Move;

//Struct to represent positions of each colour's pieces on a bitboard.
//Represents all the moves to make
//Maintaining the subset from PROMOTE_KNIGHT to PROMOTE_QUEEN is important since they are for loop bounds 
enum MoveType {NORMAL,CAPTURE,ENPASSANT,CASTLING,PROMOTE_KNIGHT,PROMOTE_BISHOP,PROMOTE_ROOK,PROMOTE_QUEEN};

//I needed a list of all the bitboard memory locations to make the move making function more simple
void Pieces_B::generate_bitboards(){
    all_bitboards.clear();
    all_bitboards.push_back(&pawns);
    all_bitboards.push_back(&bishops);
    all_bitboards.push_back(&king);
    all_bitboards.push_back(&queens);
    all_bitboards.push_back(&rooks);
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
    knights_copy = knights;
    bishops_copy = bishops;
}

//Returns to the state before a move was made
//Will be called externally from the tree search
void Pieces_B::set_to_copys(){
    //std::cout << pawns_copy << " HERE" << std::endl;
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

//Init from parent board state and then make a move
Board_State::Board_State(Board_State* board,uint_fast16_t move){
    move_database = board->move_database;
    magic_numbers = board->magic_numbers;
    white = board->white;
    black = board->black;
    white.generate_bitboards();
    black.generate_bitboards();

    //make_move(move);
    white_to_move = !board->white_to_move;
    white.set_all_pieces();
    black.set_all_pieces();
}

//Updates castling information
void Board_State::set_castling_info(){
    uint64_t king;
    //If white could castle kingside up to now
    if (white.king_side){
        //If the king and rooks are in the right place
        if (!((white.king == 0x10) && ((white.rooks & 0x80) == 0x80))){
            white.king_side = false;
        }
    }
    if (white.queen_side){
        //If the king and rooks are in the right place
        if (!((white.king == 0x10) && ((white.rooks & 0x1) == 0x1))){
            white.queen_side = false;
        }
    }
    if (black.king_side){
        //If the king and rooks are in the right place
        if ((black.king == 0x1000000000000000) && ((black.rooks & 0x8000000000000000) == 0x8000000000000000)){
            black.king_side = false;
        }
    }
    if (black.queen_side){
        //If the king and rooks are in the right place
        if ((black.king == 0x1000000000000000) && ((black.rooks & 0x100000000000000) == 0x100000000000000)){
            black.queen_side = false;
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
    print_colour(&white);
    print_colour(&black);
}

//Deprecated - kept for the project, but no longer used.
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
    while(moves != 0){
        lsb = ffsl(moves) - 1;
        add_move(from,lsb,type);
        moves &= moves - 1;
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
    int lsb;
    //Bitscan Forward Loop
    while(piece != 0){
        //Get the lsb starting at 0
        lsb = ffsl(piece) - 1;
        //Calculate moves
        blockers = Mask[lsb] & all_blockers;
        moves = Moves[lsb][(blockers * Magic[lsb]) >> (64 - Bits[lsb])];
        //So we don't take our own pieces
        moves = (moves & ~our_pieces);
        add_moves(lsb,moves,other);
        //Remove lsb
        piece &= piece - 1;
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
    int lsb;
    //Bitscan Forward Loop
    while(piece != 0){ 
        //Get the lsb starting at 0
        lsb = ffsl(piece) - 1;
        moves = move_lookup[lsb];
        //So we don't take our own pieces
        moves = (moves & ~our_pieces);
        add_moves(lsb,moves,other);
        //Remove lsb
        piece &= piece - 1;
    }
}

void Board_State::gen_white_pawn_moves(){
    uint64_t pawns = white.pawns;
    uint64_t all_positions = get_board();
    uint64_t black_positions = black.all_pieces;
    uint64_t position;
    int lsb;
    while(pawns != 0){
        lsb = ffsl(pawns) - 1;
        position = 1ULL << lsb;
        if ((all_positions & (position << 8)) == 0){
            add_white_pawn_move(lsb,lsb + 8,NORMAL);
        }
        //Two forward
        if ((all_positions & (position << 16)) == 0 && (lsb / 8) == 1){
            add_move(lsb,lsb + 16,NORMAL);
        }
        //One forward and left/right
        if ((black_positions & (position << 9)) != 0){
            add_white_pawn_move(lsb,lsb + 9,CAPTURE);
        }
        if ((black_positions & (position << 7)) != 0){
            add_white_pawn_move(lsb,lsb + 7,CAPTURE);
        }
        pawns &= pawns - 1;

    }
    //En Passant Check
    uint64_t origin = (1ULL << (prev_move >> 10));
    uint64_t destination = (1ULL << ((prev_move & 0x3f0) >> 4));

    //If the previous move was a double pawn push and behind the pawn is clear
    if ((black.pawns & destination) != 0 & (origin / destination) == (1ULL << 16) & (((destination << 8) & get_board()) == 0)){
        if ((white.pawns & (destination << 1)) != 0){
            add_move(ffsl(destination << 1) - 1,ffsl(destination << 8) - 1,2);
        }
        if ((white.pawns & (destination >> 1)) != 0){
            add_move(ffsl(destination >> 1) - 1,ffsl(destination << 8) - 1,2);
        }
    }
}

void Board_State::gen_black_pawn_moves(){
    uint64_t pawns = black.pawns;
    uint64_t all_positions = get_board();
    uint64_t white_positions = white.all_pieces;
    uint64_t position;
    int lsb;
    while(pawns != 0){
        lsb = ffsl(pawns) - 1;
        position = 1ULL << lsb;
        if ((all_positions & (position >> 8)) == 0){
            add_black_pawn_move(lsb,lsb - 8,NORMAL);
        }
        //Two forward
        if ((all_positions & (position >> 16)) == 0 && (lsb / 8) == 6){
            add_move(lsb,lsb - 16,NORMAL);
        }
        //One forward and left/right
        if ((white_positions & (position >> 9)) != 0){
            add_black_pawn_move(lsb,lsb - 9,CAPTURE);
        }
        if ((white_positions & (position >> 7)) != 0){
            add_black_pawn_move(lsb,lsb - 7,CAPTURE);
        }
        pawns &= pawns - 1;
    }
    //En Passant Check
    uint64_t origin = (1ULL << (prev_move >> 10));
    uint64_t destination = (1ULL << ((prev_move & 0x3f0) >> 4));
    
    //If the previous move was a double pawn push and behind the pawn is clear
    if ((white.pawns & destination) != 0 & (destination / origin) == (1ULL << 16) & (((destination >> 8) & get_board()) == 0)){
        if ((black.pawns & (destination << 1)) != 0){
            add_move(ffsl(destination << 1) - 1,ffsl(destination >> 8) - 1,2);
        }
        if ((white.pawns & (destination >> 1)) != 0){
            add_move(ffsl(destination >> 1) - 1,ffsl(destination >> 8) - 1,2);
        }
    }
}

void Board_State::gen_white_castling_moves(){
    //Check if we can castle and that the space is clear
    if (white.queen_side == true & (0xe & get_board()) == 0){
        add_move(4,2,3);
    }
    if (white.king_side == true & (0x60 & get_board()) == 0){
        add_move(4,6,3);
    }
}

void Board_State::gen_black_castling_moves(){
    //Check if we can castle and that the space is clear
    if (black.queen_side == true & (0xe00000000000000 & get_board()) == 0){
        add_move(60,58,3);
    }
    if (black.king_side == true & (0x6000000000000000 & get_board()) == 0){
        add_move(60,62,3);
    }
}

//Works out whose move it is, then generates that move.
void Board_State::gen_moves(){
    if (white_to_move){
        gen_white_moves();
    }
    else{
        gen_black_moves();
    }
}


//Generates all the moves for white
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
    gen_white_castling_moves();
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
    gen_black_castling_moves();
}

//Separate functions for adding pawn moves are needed in order to handle promotions
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

    // if (move_index % 32 == 0){
    //     Moves.resize(move_index + 32);
    // }
    //std::cout << "from " << origin << " to " << destination << " move type "  << type << std::endl;
    // Moves[move_index] = type + (destination << 4) + (origin << 10);
    // move_index++;
    //std::cout << (type + (destination << 4) + (origin << 10)) << std::endl;
    Moves.push_back(type + (destination << 4) + (origin << 10));
}

bool Board_State::black_king_attacked(){
    int pos = ffsl(black.king) - 1;
    uint64_t check = 0;
    uint64_t blockers, moves;

    //Check knight moves
    check |= move_database->kn_moves[pos] & white.knights;

    //Check Pawns
    check |= white.pawns & ((black.king >> 9) + (black.king >> 7));

    //Check King
    check |= move_database->ki_moves[pos] & white.king;

    //The way we check if the king is attacked by bishops, rooks and queens
    //is by pretending that the king is each of these pieces sequentially,
    //then seeing if we can attack any of the other pieces from the king.

    //Check Rooks/Queens
    blockers = (get_board() ^ (white.rooks | white.queens)) & move_database->r_mask[pos];
    moves = move_database->r_moves[pos][(blockers * magic_numbers->r_magic[pos]) >> magic_numbers->r_magic[pos]];
    check |= moves & (white.rooks | white.queens);

    //Check Bishops/Queens
    blockers = (get_board() ^ (white.bishops | white.queens)) & move_database->b_mask[pos];
    moves = move_database->b_moves[pos][(blockers * magic_numbers->b_magic[pos]) >> magic_numbers->b_magic[pos]];
    check |= moves & (white.bishops | white.queens);

    //If check is 0 the king is not attacked
    return check;
}

bool Board_State::white_king_attacked(){
    int pos = ffsl(white.king) - 1;
    uint64_t check = 0;
    uint64_t blockers, moves;

    //Check knight moves
    check |= move_database->kn_moves[pos] & black.knights;

    //Check Pawns
    check |= black.pawns & ((white.king << 9) + (white.king << 7));

    //Check King
    check |= move_database->ki_moves[pos] & black.king;

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

bool Board_State::king_attacked(){
    if (white_to_move){
        return white_king_attacked();
    }
    else{
        return black_king_attacked();
    }
}

void Board_State::make_move(uint_fast16_t move){
    //This now becomes the previous move
    prev_move = move;
    //Using bit hackery to extract the origin and destination
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
    //update castling information
    set_castling_info();
}

bool Board_State::validate_move(uint_fast16_t move){
    //Make a move then check if the king is attack,
    //and return whether or not it is valid
    make_move(move);
    if (white_to_move){
        return !black_king_attacked();
    }
    else{
        return !white_king_attacked();
    }
}

//In order to undo the above fun    std::cout << "from " << origin << " to " << destination << std::endl;
//ction or calls to make_move() in general
//Returns bitboards to the last copy that was made
void Board_State::unmake_move(){
    white.set_to_copys();
    black.set_to_copys();
}

void Board_State::add_piece_for_eval(std::vector<float>& ans,uint64_t piece,int start){
    int lsb;
    while(piece != 0){
        lsb = ffsl(piece);
        //Set the index of the vector to 1 if the piece is there
        ans[start + lsb] = 1;
        piece &= piece - 1;
    }
}

uint64_t Board_State::flip_integer(uint64_t piece){
    //Mirrors rows so that black is swapped with white
    //(Changes endianness of bitboard)
    uint64_t ans = 0ULL;
    uint64_t mask = 0xff;
    int shift = 56;
    for (int i = 0; i < 8;i++){
        ans += (mask & piece) << shift;
        shift -= 8;
        piece >>= 8;
    }
    return ans;
}

void Board_State::get_board_for_eval(std::vector<float>& ans){
    //Add check which flips board if black
    //Takes in a vector so it can be reused, constructing one each time would be wasteful
    //It requires dimensions of (772)
    int j = 0;
    if (white_to_move){
        for (int i = 0; i < 6;i++){
            add_piece_for_eval(ans,flip_integer(*(white.all_bitboards[i])),j);
            add_piece_for_eval(ans,flip_integer(*(black.all_bitboards[i])),j + 64);
            j += 128;
        }
        ans[772 - 4] = (int)white.king_side;
        ans[772 - 3] = (int)white.queen_side;
        ans[772 - 2] = (int)black.king_side;
        ans[772 - 1] = (int)black.queen_side;
    }
    //If it is blacks move we flip the board representation
    else{
        for (int i = 0; i < 6;i++){
            add_piece_for_eval(ans,*(black.all_bitboards[i]),j);
            add_piece_for_eval(ans,*(white.all_bitboards[i]),j + 64);
            j += 128;
        }
        ans[772 - 2] = (int)black.king_side;
        ans[772 - 1] = (int)black.queen_side;
        ans[772 - 4] = (int)white.king_side;
        ans[772 - 3] = (int)white.queen_side;
    }
}


// int main(){
//     Magics* magic_numbers = Get_Magics();
//     MoveData* move_database = get_move_data(magic_numbers);
//     Board_State test("8/8/8/8/8/2K5/8/2r5" ,move_database,magic_numbers);

//     //test.white_to_move = false;
//     std::cout << test.king_attacked() << std::endl;
// }

//https://lichess.org/editor/2b2bnr/1ppPpp2/r5p1/p6p/1P6/PQ2P2N/3P1PPP/RNB1KBR1_w_-_-_0_1

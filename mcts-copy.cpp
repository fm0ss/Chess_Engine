#include"moves.hpp"
#include"cppflow/ops.h"
#include"cppflow/model.h"
#include<random>
#include<math.h>


struct TreeNode{
    //We store the moves until we need them
    std::vector<TreeNode*> children;
    TreeNode* parent = NULL;
    int playouts = 1;
    float eval;
    Board_State* board;
    float calculate_eval();
    uint_fast16_t move;
    int move_index;
    int iteration;
};

float evaluate_node(cppflow::model& model, Board_State* board){
    std::vector<float> position(772,0);
    board->get_board_for_eval(position);
    auto input = cppflow::tensor(position,{1,772});
    auto output = model({{"serving_default_dense_input:0", input}},{"StatefulPartitionedCall:0"});
    auto values = output[0].get_data<float>();
    return values[0];
}

float get_eval(cppflow::model& model, Board_State* board, uint16_t move,TreeNode* parent){
    //std::cout << board->get_board() << std::endl;
    board->make_move(move);
    //std::cout << "evaluating node" << std::endl;
    float eval = evaluate_node(model,board);
    //std::cout << "node evaluated" << std::endl;
    return eval;
}



float TreeNode::calculate_eval(){
    //Average the probabilities after it
    if (parent == NULL){
        return eval / playouts + sqrt(2);
    }
    else{
        return eval / playouts + sqrt(2 * log(parent->playouts)/playouts);
    }
}

//Random number generation
int rand_range(int min, int max){
    return min + (rand() % (max - min));
}


//Returns the index of the selected Node
int selection(std::vector<TreeNode*>& Nodes){
    int max_index = 0;
    float max_eval = 0;
    float eval;
    for(int i = 0; i < Nodes.size();i++){
        eval = Nodes[i]->calculate_eval();
        if (eval > max_eval){
            max_index = i;
            max_eval = eval;
        }
    }
    return max_index;
}               


//Creates a New Node
TreeNode* expansion(TreeNode* node){
    //Select a random child to playout from
    int move_index = rand_range(0,node->board->Moves.size());
    TreeNode* child = new TreeNode();
    child->move = node->board->Moves[move_index];
    child->move_index = move_index;
    //Create the new child node
    child->board = new Board_State(node->board,node->board->Moves[move_index]);
    //Remove the move from the vector
    node->board->Moves.erase(node->board->Moves.begin() + move_index);
    //Link the new node as a child node
    node->children.push_back(child);
    child->parent = node;
    //Return so the position can be evaluated for this new node
    return child;
}

//Simulation (evaluate the node)

void simulation(TreeNode* node,cppflow::model model,int move,bool white_to_move){
    float eval = get_eval(model,node->board,move,node->parent);
    // if (node->board->white_to_move){
    //         node->eval = eval;
    // }
    // else{
    //     //If the other persons move is good for them then it is bad for you
    //     node->eval = 1 - eval;
    // }
    node->eval = eval;
    std::cout << eval << std::endl;
    // std::cout << node->eval << std::endl;
    // std::cout << node->board->get_board() << std::endl;
    //std::cout << node->board->get_board() << std::endl;
    node->board->gen_moves();
}

//Backpropagation

void backpropagation(TreeNode* leaf,bool white_to_move){
    auto current_node = leaf;
    float eval = leaf->eval;
    while (current_node->parent != NULL){
        current_node = current_node->parent;
        current_node->eval += eval;
        current_node->playouts += 1;
        eval = -eval;
    }
}

TreeNode* mcts(cppflow::model& model,Magics* magic_numbers, MoveData* move_database, Board_State& board,int iterations){
    //setup root node
    int index;
    TreeNode* root_node = new TreeNode();
    std::vector<TreeNode*> nodes = {root_node};
    root_node->board = &board;
    root_node->eval = evaluate_node(model,root_node->board);
    root_node->board->gen_moves();
    bool white_to_move = board.white_to_move;
    for (int i = 0; i < iterations;i++){
        //std::cout << i << std::end
        // std::cout << root_node->eval << " root eval" << std::endl;
        // std::cout << "iteration " << i << std::endl;
        //selection
        if (root_node->children.size() > 0){
            index = 0;
        }
        else{
            index = selection(nodes);
        }
        //std::cout << "selection complete" << std::endl;
        auto parent_node = nodes[index];
        // for (auto k : nodes){
        //     if (k->board->king_attacked()){
        //         std::cout << "king attacked" << std::endl;
        //     }
        //     if (k->board->white.king == 0 || k->board->black.king == 0){
        //         std::cout << "king missing" << std::endl;
        //         k->parent->board->print_bitboard();
        //         k->parent->board->white_to_move = !k->parent->board->white_to_move;
        //         std::cout << k->parent->board->king_attacked() << std::endl;
        //         std::cout << k->parent->board->white_to_move << std::endl;
        //         std::cout << k->parent->iteration << std::endl;
        //         std::cout << k->move << std::endl;
        //         std::cin >> wait;
        //     }
        // }
        //std::cout << "selection complete" << std::endl;
        if (parent_node->board->Moves.size() != 0){
            auto next_node = expansion(parent_node);
            next_node->iteration = i;
            simulation(next_node,model,next_node->move,white_to_move);
            // std::cout << next_node->board->white_king_attacked() << std::endl;
            // std::cout << next_node->board->black_king_attacked() << std::endl;
            // std::cout << next_node->board->get_board() << std::endl;
            if (next_node->board->king_attacked()){
                //This means that the position is after an illegal move
                //Remove the illegal move and start the iteration again
                delete next_node->board;
                delete next_node;
                i--;
                continue;
            }
            else{
                //std::cout << "simulation complete" << std::endl;
                backpropagation(next_node,white_to_move);
                nodes.push_back(next_node);
            }
        }
        else{
            //std::cout << "here I am " << std::endl;
            //If a node is fully explored remove it from the list of nodes
            nodes.erase(nodes.begin() + index);
            //check that this position isn't a win for either player
            if(parent_node->children.size() == 0 && parent_node->board->king_attacked()){
                std::cout << "here" << std::endl;
                if (parent_node->board->white_to_move == board.white_to_move){
                    //change = -current ( loss )
                    //Need to update other layers too
                    parent_node->eval = -parent_node->eval;
                    backpropagation(parent_node,white_to_move);
                    parent_node->eval = 0;
                }
                else{
                    //change = 1 - current_eval ( win )
                    //Need to update other layers too
                    parent_node->eval = 1 - parent_node->eval;
                    backpropagation(parent_node,white_to_move);
                    parent_node->eval = 1;
                }
            }
        }
        // auto next_node = expansion(parent_node);
        // simulation(next_node,model,next_node->move,position);
        // backpropagation(next_node);
    }
    return root_node;
}



int main(){

    // auto test = model.get_operations();

    // for (auto i : test){
    //     std::cout << i << std::endl;
    // }
    //Initialisation
    cppflow::model model("AI/model");
    Magics* magic_numbers = Get_Magics();
    MoveData* move_database = get_move_data(magic_numbers);
    Board_State board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",move_database,magic_numbers);

    //Get the board position
    //board.gen_white_moves();
    // std::vector<float> position(772,0);
    // for(auto i : board.white.all_bitboards){
    //     std::cout << *i << std::endl;
    // }
    // std::cout << "Eval " << get_eval(model,board,15856,position) << " Move " << 15856 << std::endl;
    // //board.unmake_move();
    // for(auto i : board.white.all_bitboards){std::cout << RootNode->children[0]->board->white_to_move << std::endl;
    //     std::cout << *i << std::endl;
    // }

    // std::cout << board.white.pawns << std::endl;



    // for (auto move : board.Moves){
    //     if (move == 0){
    //         break;
    //     }
    //     std::vector<float> position(772,0);
    //     std::cout << "Eval " << get_eval(model,board,move,position) << " Move " << move << std::endl;
    // }


    //Tree Search Here

    TreeNode* RootNode = new TreeNode();
    std::vector<TreeNode*> Nodes = {RootNode};
    std::vector<float> position(772,0);
    RootNode->board = &board;
    // board.white.king_side = false;
    // board.white.queen_side = false;
    // board.black.king_side = false;
    // board.black.queen_side = false;

    // board.gen_moves();std::cout << RootNode->children[0]->board->white_to_move << std::endl;
    // std::cout << board.Moves[8] << std::endl;
    // std::cout << get_eval(model,&board,board.Moves[8]) << std::endl;

    //One iteration
    // auto parent_node = Nodes[selection(Nodes)];
    // auto next_node = expansion(parent_node);
    // simulation(next_node,model,next_node->move,position);
    // backpropagation(next_node);
    //std::cout << evaluate_node(model,&board) << std::endl;
    RootNode = mcts(model,magic_numbers,move_database,board,10000);
    std::cout << RootNode->children.size() << std::endl;
    // std::cout << RootNode->eval << std::endl;
    // std::cout << RootNode->children[0]->eval << std::endl;
    // std::cout << RootNode->children[1]->eval << std::endl;
    for (auto node : RootNode->children){
        std::cout << node->eval << std::endl;
        std::cout << node->move << std::endl;
    }


}
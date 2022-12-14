#include"moves.hpp"
#include"cppflow/ops.h"
#include"cppflow/model.h"
#include<random>
#include<math.h>


struct TreeNode{
    std::vector<TreeNode*> children;
    TreeNode* parent = NULL;
    float playouts = 1;
    float eval;
    Board_State* board;
    float calculate_eval();
    uint16_t move;
    int move_index;
    int iteration;
};

float evaluate_node(cppflow::model& model, Board_State* board,std::vector<float>& position){
    //Initialise input vector
    std::fill(position.begin(),position.end(),0);
    //Populate it
    board->get_board_for_eval(position);
    //Get output from cppflow
    auto input = cppflow::tensor(position,{1,772});
    auto output = model({{"serving_default_dense_input:0", input}},{"StatefulPartitionedCall:0"});
    auto values = output[0].get_data<float>();
    return values[0];
}

float get_eval(cppflow::model& model, Board_State* board, uint16_t move,TreeNode* parent,std::vector<float>& position){
    //Make the move
    board->make_move(move);
    //Get the evaluation
    float eval = evaluate_node(model,board,position);
    return pow(eval,10);
}



float TreeNode::calculate_eval(){
    //This would be the root node
    //The rootnode evaluation doesn't matter since it has no selection competition
    if (parent == NULL){
        return eval + sqrt(2);
    }
    else{
        return eval + sqrt(2 * log(parent->playouts)/playouts);
    }
}

//Random number generation
int rand_range(int min, int max){
    return min + (rand() % (max - min));
}


//Returns the index of the selected Node
int sub_selection(std::vector<TreeNode*>& Nodes){
    //Initialization
    int max_index = 0;
    float max_eval = Nodes[0]->calculate_eval();
    float eval;
    //Search for maximum eval
    for(int i = 1; i < Nodes.size();i++){
        //Evaluate node
        eval = Nodes[i]->calculate_eval();
        if (eval > max_eval){
            max_index = i;
            max_eval = eval;
        }
    }
    return max_index;
}               

TreeNode* selection(TreeNode* tree_node){
    //Setting up the tree search
    TreeNode* curr_node = tree_node;
    //If the current node has not been played out then play it out
    if(curr_node->board->Moves.size() != 0){
        return curr_node;
    }
    else{
        //If this is true then we have found a good node
        if (curr_node->children.size() != 0){
            //If we haven't found a node to evaluate
            //We find the best child node from the current one
            auto index = sub_selection(curr_node->children);
            //Use of the recursive function isn't that bad since the overhead will
            //be proportional to the depth which can not frequently be that high
            return selection(curr_node->children[index]);
        }
        //Here we return a dead node which can not further be evaluated
        //This is dealt with externally
        else{
            return curr_node;
        }
    }
}


//Creates a New Node
TreeNode* expansion(TreeNode* node){
    //Select a random child to playout from
    TreeNode* child = new TreeNode();
    int move_index = rand_range(0,node->board->Moves.size());
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
void simulation(TreeNode* node,cppflow::model model,int move,std::vector<float>& position){
    //Evaluate position
    float eval = get_eval(model,node->board,move,node->parent,position);
    //Set the eval
    node->eval = eval;
    //Generate moves for the position
    node->board->gen_moves();
}

//Backpropagation
void backpropagation(TreeNode* leaf){
    //Takes the direct root from the leaf node to the root one
    auto current_node = leaf;
    float eval = leaf->eval;
    //Stop when we reach the root node
    while (current_node->parent != NULL){
        //Find the next node
        current_node = current_node->parent;
        //Update evals and playouts
        current_node->eval += eval;
        current_node->playouts += 1;
        //Alternate eval since it is a two player game
        //(the opponent makes moves which are bad for you)
        eval = -eval;
    }
}

void backpropagation_change(TreeNode* leaf){
    auto current_node = leaf;
    float eval = leaf->eval;
    while (current_node->parent != NULL){
        current_node = current_node->parent;
        current_node->eval += eval;
        eval = -eval;
    }
}

TreeNode* mcts(cppflow::model& model, Board_State& board,int iterations){
    std::vector<float> position(772,0);
    //setup root node
    int index;
    TreeNode* root_node = new TreeNode();
    root_node->board = &board;
    root_node->eval = evaluate_node(model,root_node->board,position);
    root_node->board->gen_moves();
    bool white_to_move = board.white_to_move;
    TreeNode* parent_node;
    for (int i = 0; i < iterations;i++){
        //selection
        if (root_node->board->Moves.size() > 0){
            parent_node = root_node;
        }
        else{
            parent_node = selection(root_node);
        }
        for (int j = 0; j < (int)parent_node->board->Moves.size();j++){
            if (parent_node->board->Moves.size() != 0){
            auto next_node = expansion(parent_node);
            simulation(next_node,model,next_node->move,position);
            if (next_node->board->king_attacked()){
                //This means that the position is after an illegal move
                //Remove the illegal move and start the iteration again
                parent_node->children.pop_back();
                delete next_node->board;
                delete next_node;
                next_node->board = NULL;
                next_node = NULL;
                //i--;
            }
            else{
                backpropagation(next_node);
                //nodes.push_back(next_node);
            }
        }
        else{
            //If a node is fully explored remove it from the list of nodes
            //check that this position isn't a win for either player
            //Checkmate
            if(parent_node->children.size() == 0 && parent_node->board->king_attacked()){
                parent_node->eval = 1 - parent_node->eval;
            }
            //Stalemate
            else{
                parent_node->eval = -parent_node->eval;
            }
            backpropagation_change(parent_node);

            for(int k = 0; k < parent_node->parent->children.size();k++){
                if (parent_node->parent->children[k] == parent_node){
                    //Remove link to the dead node
                    parent_node->parent->children.erase(parent_node->parent->children.begin() + k);
                }
            }
            //Finally delete the dead node
            delete parent_node->board;
            delete parent_node;
            
        }
        }
    }
    return root_node;
}

void cleanup(TreeNode* rootnode){
    //DFS
    for (auto node : rootnode->children){
        cleanup(node);
    }
    delete rootnode->board;
    delete rootnode;
}

uint16_t generate_move(cppflow::model& model, Board_State& board,int iterations){
    //Do the monte carlo tree search
    auto root_node = mcts(model,board,iterations);

    //Search for the "best" move
    float max_eval = root_node->children[0]->eval;
    uint16_t move = root_node->children[0]->move;
    for (auto node : root_node->children){
        if (node->eval > max_eval){
            max_eval = node->eval;
            move = node->move;
        }
        //Cleanup the tree
        cleanup(node);
    }
    return move;
}



int main(){
    //Initialisation
    cppflow::model model("AI/model");
    Magics* magic_numbers = Get_Magics();
    MoveData* move_database = get_move_data(magic_numbers);
    Board_State board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",move_database,magic_numbers);

    int a;
    uint16_t our_move;
    for (int i = 0; i < 100; i++){
        auto move = generate_move(model,board,100000);
        std::cout << move << "," << std::endl;
        board.make_move(move);
        std::cin >> our_move;
        board.make_move(our_move);
    }
}
//parent_node->board->Moves.size()//Get the board position
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
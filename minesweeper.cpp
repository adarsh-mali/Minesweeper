/*{Introduction}*/

//#include "../minesweeper_solver/solver.hpp"

#include<algorithm>
#include<cctype>
#include<deque>
#include<iostream>
#include<random>
#include<sstream>
#include<string>
//#include<terminos.h>
#include<utility>
#include<vector>

struct cell{
    bool hidden;
    bool mine;
    int hint;
};

static const char hint_char_set[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8'};

enum {EASY, MEDIUM, HARD};

const std::pair<int,int> easy_dem = {8,8};
const int easy_mine_no = 10;

const std::pair<int,int> med_dem = {16,16};
const int med_mine_no = 40;

const std::pair<int,int> hard_dem = {16,30};
const int hard_mine_no = 99;

bool automatic = false;
int num_rounds;
int diff = HARD;

int max_mines;
int hidden_cells;
int grid_nrows;
int grid_ncols;

std::vector<std::vector<cell>> grid;

bool game_won;
bool game_lost;
bool first_move;

void reveal_adjacent_safe_cells(int x, int y){
    std::deque<std::pair<int, int>> queue{std::pair<int, int>(x,y)};
    std::vector<std::pair<int,int>> visited;

    auto already_visisted_or_in_queue = [&visited, &queue](std::pair<int, int> index) -> bool {
        for(std::pair<int,int>& v : visited) if(v == index) return true;
        for(std::pair<int,int>& v : queue) if(v == index) return true;
        return false;
    };
}

std::vector<std::pair<int,int>> get_adjacent_indexes(int x, int y){
    int cur_x = 0, cur_y = 0;
    std::vector<std::pair<int,int>> indexes;

    for(int offset_x = -1; offset_x<2; ++offset_x){
        for(int offset_y = -1; offset_y<2; ++offset_y){
            cur_x = x + offset_x;
            cur_y = y + offset_y;

            if(cur_x >= 0 && cur_x < grid_nrows && cur_y >= 0 && cur_y < grid_ncols && !(cur_x ==x && cur_y == y)){
                indexes.push_back(std::pair<int, int>(cur_x,cur_y));
            }
        }
    }

    return indexes;
}

int get_num_adjacent_mines(int x, int y){
    int total = 0;
    std::vector<std::pair<int,int>> adjacent_indexes = get_adjacent_indexes(x, y);

    for(std::pair<int,int> index : adjacent_indexes){
        if(grid[index.first][index.second].mine) ++total;
    }

    return total;

}

void make_first_move(int x, int y){
    int row, col;
    int cur_mines = 0;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> row_rand(0,grid_nrows - 1);
    std::uniform_int_distribution<int> col_rand(0,grid_ncols - 1);

    while(cur_mines<max_mines){
        row = row_rand(gen);
        col = col_rand(gen);

        if(!grid.at(row).at(col).mine && !(row == x && col == y)){
            grid.at(row).at(col).mine = true;
            ++cur_mines;
        }
    }

    for(int row = 0; row < grid_nrows; ++row){
        for(int col = 0; col<grid_ncols; ++col){
            if(!grid.at(row).at(col).mine){
                grid.at(row).at(col).hint = get_num_adjacent_mines(row,col);
            }
        }
    }


}

void make_move(int x, int y){
    if(first_move){
        first_move = false;
        make_first_move(x,y);
        reveal_adjacent_safe_cells(x,y);
    }
}

std::vector<std::vector<int>> convert_to_simple_grid(){

    std::vector<std::vector<int>> simple(grid.size(), std::vector<int>(grid[0].size()));

    for(size_t row = 0; row<grid.size(); ++row){
        for(size_t col = 0; col<grid[0].size(); ++col){
            //What about mines??
            if(grid.at(row).at(col).hidden) simple.at(row).at(col) = -1;
            else simple.at(row).at(col) = grid.at(row).at(col).hint;
        }
    }

    return simple;

}

void init_grid(int nrows, int ncols, int num_mines){
    max_mines = num_mines;
    grid_nrows = nrows;
    grid_ncols = ncols;

    grid = std::vector<std::vector<cell>> (nrows);

    for(int row = 0; row<grid_nrows; ++row){
        grid.at(row) = std::vector<cell>(ncols);
        for(int col = 0; col<ncols; ++col){
            grid.at(row).at(col) = cell{true,false, 0};
        }
    }

    game_won = false;
    game_lost = false;
    first_move = true;
    hidden_cells = nrows*ncols;

}

void manual_play(int width, int height, int num_mines){

}

void auto_play(int width, int height, int num_mines){

    Solver s;
    std::pair<int,int> move;

    int wins = 0;
    int losses = 0;

    for(int round = 0; round<num_rounds; ++round){
        init_grid(width, height, num_mines);

        while(1){
            move = s.best_move(convert_to_simple_grid(), num_mines);
            make_move(move.first, move.second);
        }
    }

}

void print_usage_and_exit(){
    std::cout<<"Optional args: -[a/A] #NO_OF_ROUNDS, -[easy/med/hard]"<<std::endl;
    exit(0);
}

void process_arg(int argc, char **args){

    std::string ip;

    for(int i = 1; i<argc; ++i){
        ip.assign(args[i]);

        if(ip == "-a" || ip == "-A"){
            automatic = true;

            if(++i >= argc || ip.assign(args[i]).find_first_not_of("0123456789") != std::string::npos){
                print_usage_and_exit();
            }

            num_rounds = std::stoi(ip);
        } else if(ip == "-easy") diff == EASY;
        else if(ip == "-med") diff == EASY;
        else if(ip == "-hard") diff == HARD;
        else print_usage_and_exit();
        
    }

}

int main(int argc, char **args){

    process_arg(argc, args);

    int nrows, ncols, num_mines;

    if(diff == EASY){
        nrows = easy_dem.first;
        ncols = easy_dem.second;
        num_mines = easy_mine_no;
    }else if(diff == MEDIUM){
        nrows = med_dem.first;
        ncols = med_dem.second;
        num_mines = med_mine_no;
    }else{
        nrows = hard_dem.first;
        ncols = hard_dem.second;
        num_mines = hard_mine_no;
    }

    if(automatic) auto_play(nrows,ncols,num_mines);
    else manual_play(nrows,ncols,num_mines);

    return 0;

}
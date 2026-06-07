/*
    Small program to play a game of Minesweeper.

    A game can be played manually or automatically using the Solver.

    Launch using: ./MinesweeperSolver.exe -[easy/med/hard]                          for a manual game
    Launch using: ./MinesweeperSolver.exe -a [number of games] -[easy/med/hard]     for a given number of games to be played automatically by the solver, with statistics at the end.

    In a manual game, when prompted for a move type "m" and press enter. Then give a move as "row col", such as "2 5" for row 2, column 5. Enter anything other than "m" for the solver to make a move.
*/

#include "../Solver/solver.hpp"

#include <algorithm>
#include <cctype>
#include <deque>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <termios.h>
#include <utility>
#include <vector>

struct Cell{
    bool hidden;
    bool mine;
    int hint;
};

// Used for printing out hints.
static const char hint_character_set[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8'};

enum {EASY, MEDIUM, HARD};

const std::pair<int, int> EASY_DIMENSIONS{8, 8};
const int EASY_NUM_MINES = 10;

const std::pair<int, int> MEDIUM_DIMENSIONS{16, 16};
const int MEDIUM_NUM_MINES = 40;

const std::pair<int, int> HARD_DIMENSIONS{16, 30};
const int HARD_NUM_MINES = 99;


bool automatic = false;
int num_rounds;
int difficulty = HARD;

int max_mines;
int hidden_cells;
int grid_nrows;
int grid_ncols;
std::vector<std::vector<Cell> > grid;

bool game_won;
bool game_lost;
bool first_move;

void DEBUG_print()
{
    for(int row = 0; row < grid_nrows; ++row)
    {
        for(int col = 0; col < grid_ncols; ++col)
        {
            if(grid.at(row).at(col).mine)
            {
                std::cout << "M ";
            }
            else
            {
                std::cout << hint_character_set[grid.at(row).at(col).hint] << " ";
            }
        }
        std::cout << std::endl;
    }
}

std::vector<std::pair<int, int> > get_adjacent_indexes(int x, int y)
{
    int cur_x = 0, cur_y = 0;
    std::vector<std::pair<int, int> > indexes;

    for(int offset_x = -1; offset_x < 2; ++offset_x)
    {
        for(int offset_y = -1; offset_y < 2; ++offset_y)
        {
            cur_x = x + offset_x;
            cur_y = y + offset_y;

            // Make sure that we are not going out-of-bounds and are not checking self
            if( cur_x >= 0 && cur_x < grid_nrows  &&
                cur_y >= 0 && cur_y < grid_ncols &&
                !(cur_x == x && cur_y == y))
                {
                    indexes.push_back(std::pair<int, int>(cur_x, cur_y));
                }
        }
    }
    return indexes;
}

// Obtain number of mines that are adjacent to Cell at given coordinates
int get_num_adjacent_mines(int x, int y)
{
    int total = 0;
    std::vector<std::pair<int, int> > adjacent_indexes = get_adjacent_indexes(x, y);

    for(std::pair<int, int> index : adjacent_indexes)
    {
        if(grid[index.first][index.second].mine)
        {
            ++total;
        }
    }

    return total;
}

// Reveals starting Cell, and continues revealing all hint Cells of value 0.
void reveal_adjacent_safe_cells(int x, int y)
{
    std::deque<std::pair<int, int> > queue{std::pair<int, int>(x, y)};
    std::vector<std::pair<int, int> > visited;

    auto already_visisted_or_in_queue = [&visited, &queue](std::pair<int, int> index) -> bool {

        for(std::pair<int, int>& v : visited)
        {
            if(v == index)
                return true;
        }
        for(std::pair<int, int>& v : queue)
        {
            if(v == index)
                return true;
        }
        return false;
    };

    auto add_adjacent_hint_cells_to_queue = [&queue, &already_visisted_or_in_queue](std::pair<int, int> cell) -> void {

        std::vector<std::pair<int, int> > adjacent_indexes = get_adjacent_indexes(cell.first, cell.second);

        for(std::pair<int, int> index : adjacent_indexes)
        {
            if(!grid.at(index.first).at(index.second).mine && !already_visisted_or_in_queue(index))
            {
                queue.push_back(index);
            }
        }
    };

    while(!queue.empty())
    {
        std::pair<int, int> index = queue.front();
        queue.pop_front();
        visited.push_back(index);
        grid.at(index.first).at(index.second).hidden = false;
        --hidden_cells;
        if(grid.at(index.first).at(index.second).hint == 0)
            add_adjacent_hint_cells_to_queue(index);
    }
}

// Make first move of the game, marking initial cell and generating mines and hints.
// Ensures that the cell chosen as the initial move of the game is not a mine so that the player can not lose on the first turn.
void make_first_move(int initial_x, int initial_y)
{
    int row, col;
    int cur_mines = 0;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> row_rand(0, grid_nrows - 1);
    std::uniform_int_distribution<int> col_rand(0, grid_ncols - 1);

    // Randomly assign Cells to contain mines
    while(cur_mines < max_mines)
    {
        row = row_rand(gen);
        col = col_rand(gen);

        // If the current cell is not already a mine, or was the cell picked for the initial move
        if( ! grid.at(row).at(col).mine && !(row == initial_x && col == initial_y))
        {
            grid.at(row).at(col).mine = true;
            ++cur_mines;
        }
    }

    // Assign the value of hint Cells according to the number of mines adjacent to them
    for(int row = 0; row < grid_nrows; ++row)
    {
        for(int col = 0; col < grid_ncols; ++col)
        {
            if(!grid.at(row).at(col).mine)
            {
                grid.at(row).at(col).hint = get_num_adjacent_mines(row, col);
            }
        }
    }

}

// Generate a new grid of Minesweeper.
void init_grid(int nrows, int ncols, int num_mines)
{
    max_mines = num_mines;
    grid_nrows = nrows;
    grid_ncols = ncols;

    // Assign each index of the grid a Cell
    grid = std::vector<std::vector<Cell> >(nrows);
    for(int row = 0; row < nrows; ++row)
    {
        grid.at(row) = std::vector<Cell>(ncols);
        for(int col = 0; col < ncols; ++col)
        {
            grid.at(row).at(col) = Cell{true, false, 0};
        }
    }

    game_won = false;
    game_lost = false;
    first_move = true;
    hidden_cells = nrows * ncols;
}

// Reveals all Cells
void reveal_grid()
{
    for(int row = 0; row < grid_nrows; ++row)
    {
        for(int col = 0; col < grid_ncols; ++col)
        {
            grid.at(row).at(col).hidden = false;
        }
    }
}

void make_move(int x, int y)
{
    if(first_move)
    {
        first_move = false;
        make_first_move(x, y);
        reveal_adjacent_safe_cells(x, y);
    }
    else if(grid.at(x).at(y).mine)
    {
        game_lost = true;
        reveal_grid();
    }
    else
    {
        reveal_adjacent_safe_cells(x, y);
    }

    if(max_mines >= hidden_cells)
    {
        game_won = true;
        reveal_grid();
    }
}

// Get desired move from user
std::pair<int, int> get_move()
{
    std::pair<int, int> move;
    std::string input;
    std::istringstream stream;

    std::cin.ignore(1);

    std::getline(std::cin, input);
    stream = std::istringstream(input);
    stream >> move.first >> move.second;

    return move;
}

void print_grid()
{
    for(int row = 0; row < grid_nrows; ++row)
    {
        for(int col = 0; col < grid_ncols; ++col)
        {
            if(grid.at(row).at(col).hidden)
            {
                std::cout << "# ";
            }
            else if(grid.at(row).at(col).mine)
            {
                std::cout << "M ";
            }
            else
            {
                std::cout << hint_character_set[grid.at(row).at(col).hint] << " ";
            }
        }
        std::cout << std::endl;
    }
}

// Converts the current game board that consists of Cells into a simpler one consisting of ints for interfacing with the Solver.
std::vector<std::vector<int> > convert_to_simple_grid()
{
    std::vector<std::vector<int> > simple(grid.size(), std::vector<int>(grid[0].size()));

    for(size_t row = 0; row < grid.size(); ++row)
    {
        for(size_t col = 0; col < grid[0].size(); ++col)
        {
            if(grid.at(row).at(col).hidden)
            {
                simple.at(row).at(col) = -1;
            }
            else
            {
                simple.at(row).at(col) = grid.at(row).at(col).hint;
            }
        }
    }

    return simple;
}

// Automatically play desired number of games, getting all moves from the Solver.
void auto_play(int width, int height, int num_mines)
{
    Solver s;
    std::pair<int, int> move;

    int wins = 0;
    int losses = 0;

    for(int round = 0; round < num_rounds; ++round)
    {
        init_grid(width, height, num_mines);

        while(1)
        {
            move = s.best_move(convert_to_simple_grid(), num_mines);
            make_move(move.first, move.second);

            if(game_lost)
            {
                std::cout << round+1 << " of " << num_rounds << ": LOST\n";
                losses++;
                break;
            }
            else if(game_won)
            {
                std::cout << round+1 << " of " << num_rounds << ": WON\n";
                wins++;
                break;
            }
            
        }
    }

    std::cout << "Out of " << num_rounds << " rounds: " << wins << " wins, " << losses << " losses.\n";
}

// Play a single game manually, allowing user and Solver input.
void manual_play(int width, int height, int num_mines)
{
    Solver s;
    std::pair<int, int> move;
    

    init_grid(width, height, num_mines);
    print_grid();

    while(!game_lost && !game_won)
    {
        char c;
        std::cin >> c;
        if(c == 'm')
            move = get_move();
        else
        {
            move = s.best_move(convert_to_simple_grid(), num_mines);
            std::cout << "Move chosen was (" << move.first << ", " << move.second << ")\n";
        }
        

        make_move(move.first, move.second);
        print_grid();
    }

    if(game_lost)
    {
        std::cout << "You lost." << std::endl;
    }
    else if(game_won)
    {
        std::cout << "You won." << std::endl;
    }

}

void print_usage_and_exit()
{
    std::cout << "Optional args: -[a/A] #NUM_ROUNDS, -[easy/med/hard]" << std::endl;
    exit(0);
}

void parse_args(int argc, char **args)
{
    std::string cur;
    for(int i = 1; i < argc; ++i)
    {
        cur.assign(args[i]);

        if(cur == "-a" || cur == "-A")
        {
            automatic = true;

            if(++i >= argc || cur.assign(args[i]).find_first_not_of("0123456789") != std::string::npos)
            {
                print_usage_and_exit();
            }
            num_rounds = std::stoi(cur);
        }
        else if(cur == "-easy")
        {
            difficulty = EASY;
        }
        else if(cur == "-med")
        {
            difficulty = MEDIUM;
        }
        else if(cur == "-hard")
        {
            difficulty = HARD;
        }
        else
        {
            print_usage_and_exit();
        }
    }
}

int main(int argc, char **args)
{
    int nrows, ncols, num_mines;
    parse_args(argc, args);

    if(difficulty == EASY)
    {
        nrows = EASY_DIMENSIONS.first;
        ncols = EASY_DIMENSIONS.second;
        num_mines = EASY_NUM_MINES;
    }
    else if(difficulty == MEDIUM)
    {
        nrows = MEDIUM_DIMENSIONS.first;
        ncols = MEDIUM_DIMENSIONS.second;
        num_mines = MEDIUM_NUM_MINES;
    }
    else
    {
        nrows = HARD_DIMENSIONS.first;
        ncols = HARD_DIMENSIONS.second;
        num_mines = HARD_NUM_MINES;
    }

    if(automatic)
    {
        auto_play(nrows, ncols, num_mines);
    }
    else
    {
        manual_play(nrows, ncols, num_mines);
    }
    
    return 0;
}
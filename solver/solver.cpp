#include "solver.hpp"

#include "matrix.hpp"

#include <cmath>
#include <map>
#include <random>
#include <utility>
#include <vector>

#include <iostream>


int Solver::count_nonzero_hints(Matrix& board)
{
    int count = 0;

    for(int row = 0; row < board.height; ++row)
    {
        for(int col = 0; col < board.width; ++col)
        {
            if(board(row, col) > 0)
            {
                ++count;
            }
        }
    }

    return count;
}

int Solver::count_hidden_cells(Matrix& board)
{
    int count = 0;

    for(int row = 0; row < board.height; ++row)
    {
        for(int col = 0; col < board.width; ++col)
        {
            if(board(row, col) == -1)
            {
                ++count;
            }
        }
    }

    return count;
}

// Use known mine locations and mark cells accordingly. Then subtract 1 from hint cells adjacent to those mines.
void Solver::normalize_board(Matrix& board, std::map<std::pair<int, int>, bool>& known_mines)
{
    for(auto it = known_mines.begin(); it != known_mines.end(); ++it)
    {
        board(it->first.first, it->first.second) = -2;

        std::vector<std::pair<int, int> > adjacent_indecies = board.get_adjacent_indices(it->first.first, it->first.second);
        for(std::pair<int, int> index : adjacent_indecies)
        {
            if(board(index.first, index.second) > 0)
            {
                --board(index.first, index.second);
            }
        }
    }
}

/*
    Each column of the logic matrix, except for the last, correlates to a frontier cell. The integers in the last column are the values of the hint cells which those frontier cells are
    adjacent to. These correlations are kept track of with a FrontierMap.
*/
Matrix Solver::construct_logic_matrix(Matrix& board, FrontierMap& fmap)
{
    int count = 0;
    Matrix unsolved_matrix(count_nonzero_hints(board), fmap.size()+1);

    for(int row = 0; row < board.height; ++row)
    {
        for(int col = 0; col < board.width; ++col)
        {
            //If this cell is a hint
			if (board(row, col) > 0)
			{
				bool onFringe = false;
				//Find all adjacent cells that are fringe cells related to this hint
                		std::vector<std::pair<int, int> > adjacent_indices = board.get_adjacent_indices(row, col);
				for(std::pair<int, int> index : adjacent_indices)
                		{
                    			//If adjacent cell is "unknown"
                    			if(board(index.first, index.second) == -1)
                    			{
                        			onFringe = true;
                        			std::pair<int, int> pos(index.first, index.second);
						unsolved_matrix(count, fmap(pos)) = 1;
                    			}
               			}
				if (onFringe)
				{
					unsolved_matrix(count++, unsolved_matrix.width - 1) = board(row, col);
				}
			}
        }
    }

    return unsolved_matrix;
}

// Find if there is a move that is guarenteed to be safe.
bool Solver::find_guaranteed_move(Matrix& board, Matrix& unsolved_logic_matrix, Matrix& solved_logic_matrix, FrontierMap& fmap, std::map<std::pair<int, int>, bool>& known_mines, std::pair<int, int>& move)
{
    // Go through each row of the solved_logic_matrix
    for(int row = 0; row < solved_logic_matrix.height; ++row)
    {
        int col;

        if((col = solved_logic_matrix.is_lonely_row(row)) != -1)
        {
            if(solved_logic_matrix(row, solved_logic_matrix.width - 1) == 0)
            {
                move = fmap(col);
                return true;
            }
            else if(solved_logic_matrix(row, solved_logic_matrix.width - 1) == 1)
            {
                known_mines[fmap(col)] = true;
            }
        }
        else if(solved_logic_matrix.is_safe_row(row))
        {
            for(int i = 0; i < solved_logic_matrix.width - 1; ++i)
            {
                if(solved_logic_matrix(row, i) == 1)
                {
                    move = fmap(i);
                    return true;
                }
            }
        }
    }

    // Check to see if there are hint cells that have the same number of adjacent hidden cells as their hint value. In theses cases, all hidden cells adjacent to the hint cell are mines.
    for(int row = 0; row < unsolved_logic_matrix.height; ++row)
    {
        if(unsolved_logic_matrix(row, unsolved_logic_matrix.width - 1) > 0)
        {
            int count = 0;
            for(int col = 0; col < unsolved_logic_matrix.width - 1; ++col)
            {
                if(unsolved_logic_matrix(row, col) != 0)
                {
                    ++count;
                }
            }
            if(count == unsolved_logic_matrix(row, unsolved_logic_matrix.width - 1))
            {
                for(int col = 0; col < unsolved_logic_matrix.width - 1; ++col)
                {
                    if(unsolved_logic_matrix(row, col) != 0)
                    {
                        known_mines[fmap(col)] = true;
                    }
                }
            }
        }
    }

    // We have found the locations of some mines, so use that to see if we can now find a guarenteed safe cell.
    normalize_board(board, known_mines);

    return find_move_from_normalized_board(board, move);
}

// Pick any random cell that is hidden as our move.
std::pair<int, int> Solver::random_move(Matrix& normalized_board)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> row_rand(0, normalized_board.height - 1);
    std::uniform_int_distribution<int> col_rand(0, normalized_board.width - 1);
    int row, col;

    while(1)
    {
        row = row_rand(gen);
        col = col_rand(gen);

        if(normalized_board(row, col) == -1)
        {
            return {row, col};
        }
    }
    return {0, 0};
}

// Calculate the probability that k mines exist within the n frontier cells, given p probability of a cell being a mine.
double Solver::binomial_pmf(int n, int k, int p)
{
    auto binomial_coeff = [&n, &k](){
        double result = 1;
        for(int i = 1; i <= k; ++i)
        {
            result *= (n + 1 - i) / i;
        }
        return result;
    };

    return binomial_coeff() * std::pow(p, k) * std::pow((1 - p), (n - k));
}

// Given a combination, return how many mines are present.
int Solver::count_num_mines_in_combo(std::vector<bool> &combo)
{
    int count = 0;
    for(size_t i = 0; i < combo.size(); ++i)
    {
        if(combo[i] == true)
        {
            count++;
        }
    }
    return count;
}

// Count the how many combinations contain certain numbers of mines.
std::map<int, int> Solver::count_combinations(std::vector<std::vector<bool> >& combinations)
{
    int mine_count_in_combo;
    std::map<int, int> combo_counts;

    for(std::vector<bool>& combo : combinations)
    {
        mine_count_in_combo = count_num_mines_in_combo(combo);

        if(combo_counts.count(mine_count_in_combo) == 0)
        {
            combo_counts[mine_count_in_combo] = 1;
        }
        else
        {
            combo_counts[mine_count_in_combo] += 1;
        }
    }

    return combo_counts;
}

// A combination is valid if it satisfies the constraints given by the hint cells.
bool Solver::is_valid_combination(Matrix& normalized_board, FrontierMap& normalized_fmap, std::vector<bool>& combo)
{
    Matrix temp_board(normalized_board);

    for(size_t i = 0; i < combo.size(); ++i)
    {
        if(combo[i] == true)
        {
            temp_board(normalized_fmap(i).first, normalized_fmap(i).second) = -2;
        }
    }

    for(int row = 0; row < temp_board.height; ++row)
    {
        for(int col = 0; col < temp_board.width; ++col)
        {
            if(temp_board(row, col) > 0)
            {
                int count = 0;
                std::vector<std::pair<int, int> > adjacent_indices = temp_board.get_adjacent_indices(row, col);
                for(std::pair<int, int>& index : adjacent_indices)
                {
                    if(temp_board(index.first, index.second) == -2)
                    {
                        count++;
                    }
                }
                if(count != temp_board(row, col))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

void Solver::generate_combinations_recursive(Matrix& normalized_board, std::vector<std::vector<bool> >& combinations, std::vector<bool>& combo, FrontierMap& normalized_fmap, size_t pos)
{

    if(depth_counter == MAX_COMBO_DEPTH)
    {
        return;
    }
    depth_counter++;

    if(pos == combo.size())
    {
        if(is_valid_combination(normalized_board, normalized_fmap, combo))
        {
            combinations.push_back(combo);
        }
        return;
    }

    combo[pos] = false;
    generate_combinations_recursive(normalized_board, combinations, combo, normalized_fmap, pos+1);

    combo[pos] = true;
    generate_combinations_recursive(normalized_board, combinations, combo, normalized_fmap, pos+1);
}

// Generate all possible combinations of mines in the frontier.
std::vector<std::vector<bool> > Solver::generate_combinations(Matrix& normalized_board, FrontierMap& normalized_fmap)
{
    std::vector<std::vector<bool> > combinations;
    std::vector<bool> combo(normalized_fmap.size());
    depth_counter = 0;
    generate_combinations_recursive(normalized_board, combinations, combo, normalized_fmap, 0);
    return combinations;
}

// After the board is normalized, a safe move may now be apparent. Check for hint cells of value 0. Any adjacent hidden cells must be safe.
bool Solver::find_move_from_normalized_board(Matrix& normalized_board, std::pair<int, int>& move)
{
    for(int row = 0; row < normalized_board.height; ++row)
    {
        for(int col = 0; col < normalized_board.width; ++col)
        {
            // If this cell has a hint value of 0
            if(normalized_board(row, col) == 0)
            {
                // Check all adjacent cells
                for(std::pair<int, int>& index : normalized_board.get_adjacent_indices(row, col))
                {
                    // If the adjacent cell is hidden, then it must be safe
                    if(normalized_board(index.first, index.second) == -1)
                    {
                        move = index;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


/*
    There are no guarenteed safe moves, so use probability to find a move that has the highest chance of being safe.

    First, calculate the probabilites of the frontier having certain amounts of mines using the binomial distribution. Then see how many combinations contain those amounts of mines.
    If many combinations exist that have a certain amount of mines, then the probability of that number of mines occuring in the frontier is split between those combinations. 

    Then we see whether there is a combination that has a higher chance of being true than the probability of any random cell being a mine. If there is, then we assume that combination to be true
    and we pick a safe cell from it. If not, then just pick any random cell as our move.
*/
void Solver::find_safest_move(Matrix& normalized_board, FrontierMap& fmap, std::map<std::pair<int, int>, bool>& known_mines, std::pair<int, int>& move, int num_max_mines)
{
    int remaining_mines;
    int remaining_cells;

    FrontierMap normalized_fmap(normalized_board);

    remaining_mines = num_max_mines - known_mines.size();
    remaining_cells = count_hidden_cells(normalized_board);

    double generic_mine_probability = remaining_mines / remaining_cells;
    std::vector<std::vector<bool> > combinations = generate_combinations(normalized_board, normalized_fmap);
    std::map<int, int> combo_counts = count_combinations(combinations);
    std::vector<double> probabilities_for_num_mines;

    // Calculate how likely it is for the frontier to contain various amounts of mines
    for(auto it = combo_counts.begin(); it != combo_counts.end(); ++it)
    {
        probabilities_for_num_mines.push_back(binomial_pmf(fmap.size(), it->first, generic_mine_probability));
    }

    // Normalize the probabilites amongst themselves, so that they add up to 100%
    double probability_total = 0.0;

    for(double &prob : probabilities_for_num_mines)
    {
        probability_total += prob;
    }

    for(double &prob : probabilities_for_num_mines)
    {
        prob /= probability_total;
    }

    double predicted_num_mines_inside_frontier = 0.0;
    int counter = 0;
    for(auto it = combo_counts.begin(); it != combo_counts.end(); ++it)
    {
        predicted_num_mines_inside_frontier += it->first * probabilities_for_num_mines[counter++];
    }
    double probability_for_mine_outside_frontier = (remaining_mines - predicted_num_mines_inside_frontier) / (remaining_cells - normalized_fmap.size());

    // Check the probability of each combo occuring
    for(auto it = combo_counts.begin(); it != combo_counts.end(); ++it)
    {
        counter = 0;
        // If this combination has a greater chance of occuring than the probability of a random outside cell being a mine, pick it
        if(probabilities_for_num_mines[counter++] / it->second >= probability_for_mine_outside_frontier)
        {
            // Find a safe cell within the combination
            for(std::vector<bool>& combo : combinations)
            {
                if(count_num_mines_in_combo(combo) == it->first)
                {
                    for(size_t i = 0; i < combo.size(); ++i)
                    {
                        if(combo[i] == false)
                        {
                            move = normalized_fmap(i);
                            return;
                        }
                    }
                }
            }
        }
    }
    move = random_move(normalized_board);
}

// Check to see if this is the first move for the game.
bool Solver::is_first_move(Matrix& board)
{
    for(int row = 0; row < board.height; ++row)
    {
        for(int col = 0; col < board.width; ++col)
        {
            if(board(row, col) != -1)
            {
                return false;
            }
        }
    }

    return true;
}

// Return the best possible move for the given board.
std::pair<int, int> Solver::best_move(std::vector<std::vector<int> > grid, int num_max_mines)
{
    std::pair<int, int> move(-1, -1);

    Matrix board(grid);

    // If this is the first move of the game, just pick the top-left cell.
    if(is_first_move(board))
    {
        return {0, 0};
    }

    FrontierMap fmap(board);
    std::map<std::pair<int, int>, bool> known_mines;
    Matrix unsolved_logic_matrix = construct_logic_matrix(board, fmap);
    Matrix solved_logic_matrix(unsolved_logic_matrix);
    solved_logic_matrix.rref();

    //unsolved_logic_matrix.print();
    //solved_logic_matrix.print();

    if(!find_guaranteed_move(board, unsolved_logic_matrix, solved_logic_matrix, fmap, known_mines, move))
    {
     
        find_safest_move(board, fmap, known_mines, move, num_max_mines);
    }

    return move;
}

/*
    Declaration of the Solver class which is used to find the best move for a given state of Minesweeper.

    The Solver first finds cells that constitute the "frontier" of a given board. These are hidden cells that are adjacent to a hint cell. A matrix is then constructed representing the relationship
    between these frontier cells and their adjacent hint cells. Computing the rref of this matrix gives information on the location of safe and mine cells in the frontier. If a safe cell is located for a given state
    it is picked as the move for the round. If not, a copy of the board is made, and known mine locations that were computed earlier are marked. If these markings reveal the location of a safe cell, then that is picked.
    If there still is no guarenteed safe cell, the solver computes the probabiities of cells along the frontier having mines in them. This is done by generating all possible combinations of mine placements. The probability
    that a cell outside the frontier contains a mine is also calculated. If it is found that there is a higher chance of one of the frontier cells containing a mine, then a random outside cell is picked. Otherwise the frontier cell
    with the least likely probability of containing a mine is picked.
*/

#pragma once

#include "frontier.hpp"
#include "matrix.hpp"

#include <map>
#include <utility>
#include <vector>

class Solver
{
    private:

    const int MAX_COMBO_DEPTH = 60000; // Limit how many combinations are generated. Higher = more time, but higher chance of success.
    int depth_counter;

    int count_nonzero_hints(Matrix& board);
    int count_hidden_cells(Matrix& board);
    void normalize_board(Matrix& board, std::map<std::pair<int, int>, bool>& known_mines);
    Matrix construct_logic_matrix(Matrix& board, FrontierMap& fmap);
    bool find_guaranteed_move(Matrix& board, Matrix& unsolved_logic_matrix, Matrix& solved_logic_matrix, FrontierMap& fmap, std::map<std::pair<int, int>, bool>& known_mines, std::pair<int, int>&  move);
    
    std::pair<int, int> random_move(Matrix& normalized_board);
    double binomial_pmf(int n, int k, int p);
    int count_num_mines_in_combo(std::vector<bool> &combo);
    std::map<int, int> count_combinations(std::vector<std::vector<bool> >& combinations);
    bool is_valid_combination(Matrix& normalized_board, FrontierMap& normalized_fmap, std::vector<bool>& combo);
    void generate_combinations_recursive(Matrix& normalized_board, std::vector<std::vector<bool> >& combinations, std::vector<bool>& combo, FrontierMap& normalized_fmap, size_t pos);
    std::vector<std::vector<bool> > generate_combinations(Matrix& normalized_board, FrontierMap& normalized_fmap);

    bool find_move_from_normalized_board(Matrix& normalized_board, std::pair<int, int>& move);
    void find_safest_move(Matrix& normalized_board, FrontierMap& fmap, std::map<std::pair<int, int>, bool>& known_mines, std::pair<int, int>& move, int num_max_mines);

    bool is_first_move(Matrix& board);

    public:
    
    std::pair<int, int> best_move(std::vector<std::vector<int> > grid, int um_max_mines);
};
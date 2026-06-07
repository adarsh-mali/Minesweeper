/*
    This class is used to keep track of which column in the logic matrix correlates to what cell on the game board.
*/

#pragma once

#include "matrix.hpp"

#include <map>
#include <utility>

class FrontierMap
{
    private:

    std::map<std::pair<int, int>, int>  pos_to_index_map;
    std::map<int, std::pair<int, int> > index_to_pos_map;

    int _count;

    public:

    FrontierMap();
    FrontierMap(const FrontierMap& other);
    FrontierMap(Matrix& board);
    ~FrontierMap();

    void add(int x, int y);
    int count(const std::pair<int, int>& coord);
    int count(const int& col);
    int size();

    int operator()(const std::pair<int, int>& coord);
    std::pair<int, int> operator()(const int& col);
};
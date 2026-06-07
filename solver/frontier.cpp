#include "frontier.hpp"

FrontierMap::FrontierMap()
{
    _count = 0;
}

FrontierMap::FrontierMap(const FrontierMap& other)
{
    pos_to_index_map = other.pos_to_index_map;
    index_to_pos_map = other.index_to_pos_map;
    _count = other._count;
}

FrontierMap::FrontierMap(Matrix& board)
{
    auto is_frontier_cell = [&board](int x, int y) -> bool {
        // If the cell is "unknown"
        if(board(x, y) == -1)
        {
            std::vector<std::pair<int, int> > neighbors = board.get_adjacent_indices(x, y);
            for(std::pair<int, int> neightbor : neighbors)
            {
                if(board(neightbor.first, neightbor.second) >= 0)
                {
                    return true;
                }
            }
        }
        return false;
    };


    _count = 0;
    
    for(int row = 0; row < board.height; ++row)
    {
        for(int col = 0; col < board.width; ++col)
        {
            if(is_frontier_cell(row, col))
            {
                add(row, col);
            }
        }
    }
}

FrontierMap::~FrontierMap()
{

}

void FrontierMap::add(int x, int y)
{
    std::pair<int, int> coord(x, y);
    pos_to_index_map[coord] = _count;
    index_to_pos_map[_count++] = coord;
}

int FrontierMap::operator()(const std::pair<int, int>& coord)
{
    return pos_to_index_map[coord];
}

std::pair<int, int> FrontierMap::operator()(const int& col)
{
    return index_to_pos_map[col];
}

int FrontierMap::count(const std::pair<int, int>& coord)
{
    return pos_to_index_map.count(coord);
}

int FrontierMap::count(const int& col)
{
    return index_to_pos_map.count(col);
}

int FrontierMap::size()
{
    return _count;
}


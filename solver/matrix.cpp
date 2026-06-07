#include "matrix.hpp"

#include <cstdlib>
#include <iostream>
#include <iomanip>

Matrix::Matrix()
{

}

Matrix::Matrix(int num_rows, int num_cols)
{
	data = std::vector<std::vector<int> >(num_rows, std::vector<int>(num_cols));
	width = num_cols;
	height = num_rows;
}

Matrix::Matrix(const std::vector<std::vector<int> >& d)
{
	data = d;
	width = d[0].size();
	height = d.size();
}

Matrix::Matrix(std::vector<std::vector<int> >&& d)
{
	data = std::move(d);
	width = data[0].size();
	height = data.size();
}

Matrix::~Matrix()
{

}

std::vector<int>& Matrix::operator()(int index)
{
	return data.at(index);
}

int& Matrix::operator()(int index1, int index2)
{
	return data.at(index1).at(index2);
}

void Matrix::swap_rows(int row1, int row2)
{
    data[row1].swap(data[row2]);
}

void Matrix::divide_row(int row, int divisor)
{
    int size = data[row].size();
    for(int i = 0; i < size; ++i)
    {
        data[row][i] /= divisor;
    }
}

void Matrix::add_row(int row1, int row2)
{
    int size = data[row1].size();
    for(int i = 0; i < size; ++i)
    {
        data[row1][i] += data[row2][i];
    }
}

void Matrix::subtract_row(int row1, int row2)
{
    int size = data[row1].size();
    for(int i = 0; i < size; ++i)
    {
        data[row1][i] -= data[row2][i];
    }
}

void Matrix::rref()
{
    int i = 0, j = 0;
    int nrows = data.size();
    int ncols = data[0].size();

    while(i < nrows && j < ncols)
    {
        // Choose a pivot
        if(data[i][j] == 0)
        {
            bool done = false;
            while(!done)
            {
                if(j >= ncols)
                {
                    swap_rows(nrows - 1, nrows - 2);
                    return;
                }
                
               for (int n = i + 1; n < nrows; ++n)
				{
					if (data[n][j] != 0)
					{
						swap_rows(i, n);
						done = true;
						break;
					}
				}
				if (!done)
				{
					j++;
					if (j >= ncols)
						return;
					if (data[i][j] != 0)
					{
						done = true;
					}
				}
            }
        }

        // Divide row by pivot value, to make pivot equal 1
		divide_row(i, data[i][j]);

		//  Zero out column using pivot
		for (int n = 0; n < nrows; ++n)
		{
			if (n != i && data[n][j] != 0)
			{
				int value = abs(data[n][j]);
				for (int k = 0; k < value; ++k)
				{
					if (data[n][j] < 0)
					{
						add_row(n, i);
					}
					else if (data[n][j] > 0)
					{
						subtract_row(n, i);
					}
				}
			}
		}
		i++;
		j++;
    }

	return;
}

std::vector<std::pair<int, int> > Matrix::get_adjacent_indices(int x, int y)
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
            if( cur_x >= 0 && cur_x < height &&
                cur_y >= 0 && cur_y < width  &&
                !(cur_x == x && cur_y == y))
                {
                    indexes.push_back(std::pair<int, int>(cur_x, cur_y));
                }
        }
    }
    return indexes;
}

// A row is lonely if there is only 1 non-zero entry besides the last one.
int Matrix::is_lonely_row(int row)
{
    int count = 0;
	int col;
	for (int i = 0; i < width - 1; ++i)
	{
		if (data[row][i] != 0)
		{
			count++;
			col = i;
			if (count > 1)
			{
				return -1;
			}
		}
	}
	if (count == 1)
	{
		return col;
	}
	return -1;
}

// A row is "safe" if the final entry is 0, there is at least one entry of value 1, and no other entries are anything other than 1 or 0.
bool Matrix::is_safe_row(int row)
{
    if (data[row][width - 1] != 0)
	{
		return false;
	}
	bool at_least_one_positive = false;
	for (int i = 0; i < width - 1; ++i)
	{
		if (data[row][i] == 1)
		{
			at_least_one_positive = true;
		}
		else if (data[row][i] != 1 && data[row][i] != 0)
		{
			return false;
		}
	}
	return at_least_one_positive;
}

// For debugging
void Matrix::print()
{
	std::cout << "\n";
	for(int row = 0; row < height; ++row)
	{
		for(int col = 0; col < width; ++col)
		{
			std::cout << std::setw(2) << data[row][col] << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}
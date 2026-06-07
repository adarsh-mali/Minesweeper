/*
    Class that represents a mathematical matrix. Contains functions to convert matrix to reduced row-echelon form.
*/

#pragma once

#include <vector>

class Matrix
{
    private:

    std::vector<std::vector<int> > data;

    void swap_rows(int row1, int row2);
    void divide_row(int row, int divisor);
    void add_row(int row1, int row2);
    void subtract_row(int row1, int row2);

    public:

    int width;
    int height;

    Matrix();
    Matrix(int num_rows, int num_cols);
    Matrix(const std::vector<std::vector<int> >& d);
    Matrix(std::vector<std::vector<int> >&& d);
    ~Matrix();

    std::vector<int>& operator()(int index);
    int& operator()(int index1, int index2);

    void rref();
    std::vector<std::pair<int, int> > get_adjacent_indices(int x, int y);
    int is_lonely_row(int row);
    bool is_safe_row(int row);

    void print();
};
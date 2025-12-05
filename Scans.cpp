#include <cstdlib>
#include <vector>
#include <iostream>
#include <fstream>
#include <dlfcn.h>
#include "RobotBase.h"

void add_if_valid(int row, int col, std::vector<RadarObj>* radar_results, std::vector<char>& board, int rows, int cols)
{
    if (row < 0 || row >= rows || col < 0 || col >= cols)
        return;
    char cell = board[row * cols + col];

    if (cell != '.') {
        if (cell == 'M' || cell == 'P' || cell == 'F' || cell=='X') {
            radar_results->emplace_back(cell, row, col);
        } else {
            radar_results->emplace_back('R', row, col); 
        }
    }
}

void up(std::vector<RadarObj> *radar_results, std::vector<char>& board, int rows, int cols, RobotBase* robot){
    (void) rows;
    int r_row, r_col;
    robot->get_current_location(r_row, r_col);
    for(int row = r_row - 1; row >= 0; row --){

        add_if_valid(row, r_col, radar_results, board, rows, cols);

        add_if_valid(row, r_col+1, radar_results, board, rows, cols);

        add_if_valid(row, r_col-1, radar_results, board, rows, cols);
    }
}
void down(std::vector<RadarObj> *radar_results, std::vector<char>& board, int rows, int cols, RobotBase* robot){
    int r_row, r_col;
    robot->get_current_location(r_row, r_col);
    
    for (int row = r_row + 1; row < rows; row++)
    {

        add_if_valid(row, r_col, radar_results, board, rows, cols);

        add_if_valid(row, r_col + 1, radar_results, board, rows, cols);

        add_if_valid(row, r_col - 1, radar_results, board, rows, cols);
    }
}
void right(std::vector<RadarObj>* radar_results, std::vector<char>& board, int rows, int cols, RobotBase* robot)
{
    int r_row, r_col;
    robot->get_current_location(r_row, r_col);

     for (int col = r_col + 1; col < cols; col++)
    {
        // center row
        add_if_valid(r_row, col, radar_results, board, rows, cols);

        // upper row
        add_if_valid(r_row - 1, col, radar_results, board, rows, cols);

        // lower row
        add_if_valid(r_row + 1, col, radar_results, board, rows, cols);
    }
}
void left(std::vector<RadarObj>* radar_results, std::vector<char>& board, int rows, int cols, RobotBase* robot)
{
    int r_row, r_col;
    robot->get_current_location(r_row, r_col);
    for (int col = r_col - 1; col >= 0; col--)
    {
        // center row
        add_if_valid(r_row, col, radar_results, board, rows, cols);

        // upper row
        add_if_valid(r_row - 1, col, radar_results, board, rows, cols);

        // lower row
        add_if_valid(r_row + 1, col, radar_results, board, rows, cols);
    }
}
void down_right(std::vector<RadarObj>* radar_results, std::vector<char>& board, int rows, int cols, RobotBase* robot)
{
    int r_row, r_col;
    robot->get_current_location(r_row, r_col);

    int row = r_row + 1;
    for (int col = r_col + 1; col < cols && row < rows; col++, row++)
        add_if_valid(row, col, radar_results, board, rows, cols);

    row = r_row;
    for (int col = r_col + 1; col < cols; col++, row++)
        add_if_valid(row, col, radar_results, board, rows, cols);

    row = r_row;
    for (int col = r_col + 2; col < cols; col++, row++)
        add_if_valid(row, col, radar_results, board, rows, cols);


    row = r_row + 1;
    for (int col = r_col; col < cols; col++, row++)
        add_if_valid(row, col, radar_results, board, rows, cols);

    row = r_row + 2;
    for (int col = r_col; col < cols; col++, row++)
        add_if_valid(row, col, radar_results, board, rows, cols);
}
void down_left(std::vector<RadarObj>* radar_results, std::vector<char>& board, int rows, int cols, RobotBase* robot)
{
    int r_row, r_col;
    robot->get_current_location(r_row, r_col);

    // middle
    int row = r_row + 1;
    for (int col = r_col - 1; col >= 0 && row < rows; col--, row++)
        add_if_valid(row, col, radar_results, board, rows, cols);

    // left 1
    row = r_row;
    for (int col = r_col - 1; col >= 0; col--, row++)
        add_if_valid(row, col, radar_results, board, rows, cols);

    // left 2
    row = r_row;
    for (int col = r_col - 2; col >= 0; col--, row++)
        add_if_valid(row, col, radar_results, board, rows, cols);

    // down 1
    row = r_row + 1;
    for (int col = r_col; col >= 0; col--, row++)
        add_if_valid(row, col, radar_results, board, rows, cols);

    // down 2
    row = r_row + 2;
    for (int col = r_col; col >= 0; col--, row++)
        add_if_valid(row, col, radar_results, board, rows, cols);
}


void up_right(std::vector<RadarObj>* radar_results, std::vector<char>& board, int rows, int cols, RobotBase* robot)
{
    int r_row, r_col;
    robot->get_current_location(r_row, r_col);

    // middle
    int row = r_row - 1;
    for (int col = r_col + 1; col < cols && row >= 0; col++, row--)
        add_if_valid(row, col, radar_results, board, rows, cols);

    // right 1
    row = r_row;
    for (int col = r_col + 1; col < cols; col++, row--)
        add_if_valid(row, col, radar_results, board, rows, cols);

    // right 2
    row = r_row;
    for (int col = r_col + 2; col < cols; col++, row--)
        add_if_valid(row, col, radar_results, board, rows, cols);

    // up 1
    row = r_row - 1;
    for (int col = r_col; col < cols; col++, row--)
        add_if_valid(row, col, radar_results, board, rows, cols);

    // up 2
    row = r_row - 2;
    for (int col = r_col; col < cols; col++, row--)
        add_if_valid(row, col, radar_results, board, rows, cols);
}

void up_left(std::vector<RadarObj>* radar_results, std::vector<char>& board, int rows, int cols, RobotBase* robot)
{
    int r_row, r_col;
    robot->get_current_location(r_row, r_col);

    // middle
    int row = r_row - 1;
    for (int col = r_col - 1; col >= 0 && row >= 0; col--, row--)
        add_if_valid(row, col, radar_results, board, rows, cols);

    // left 1
    row = r_row;
    for (int col = r_col - 1; col >= 0; col--, row--)
        add_if_valid(row, col, radar_results, board, rows, cols);

    // left 2
    row = r_row;
    for (int col = r_col - 2; col >= 0; col--, row--)
        add_if_valid(row, col, radar_results, board, rows, cols);

    // up 1
    row = r_row - 1;
    for (int col = r_col; col >= 0; col--, row--)
        add_if_valid(row, col, radar_results, board, rows, cols);

    // up 2
    row = r_row - 2;
    for (int col = r_col; col >= 0; col--, row--)
        add_if_valid(row, col, radar_results, board, rows, cols);
}

void scan_surrounding(std::vector<RadarObj>* radar_results,std::vector<char>& board,int rows, int cols,RobotBase* robot)
{
    int r_row, r_col;
    robot->get_current_location(r_row, r_col);

    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            // Skip the robot's own cell
            if (dr == 0 && dc == 0) continue;

            int row = r_row + dr;
            int col = r_col + dc;

            if (row >= 0 && row < rows && col >= 0 && col < cols) {
                if (board[row * cols + col] != '.') {
                    add_if_valid(row, col, radar_results, board, rows, cols);
                }
            }
        }
    }
}

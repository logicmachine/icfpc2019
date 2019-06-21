#include "board_loader.hpp"

#include <iostream>

using boardloader::Cell;
using boardloader::load_board;
using boardloader::print_table;

using namespace std;

int main()
{
    int start_y, start_x;
    auto table = load_board("/home/xyz600/program/contest/icfpc/2019/example/example-01.desc", start_y, start_x);

    std::cout << "start_y = " << start_y << endl;
    std::cout << "start_x = " << start_x << endl;

    Cell cell = Cell::Empty;

    cout << table[0][0] << endl;

    print_table(table, start_y, start_x);
}
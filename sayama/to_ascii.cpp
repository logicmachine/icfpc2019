#include "board_loader.hpp"

using boardloader::load_board;
using boardloader::save_ascii;

int main(int argc, char* argv[])
{
    std::string input(argv[1]);
    std::string output(argv[2]);

    int y, x;
    auto board = load_board(input, y, x);
    save_ascii(board, y, x, output);
}
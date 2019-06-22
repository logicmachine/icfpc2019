#include "bits/stdc++.h"

#include "./ikeda.hpp"

int main(int argc, char *argv[]) 
{
    if (argc != 2) return 1;
    string path(argv[1]);
    int x, y;
    auto board = boardloader::load_board(path, x, y);
    auto ret = ikeda::largest_rectangle(board);
    cout << ret.size() << endl;
    return 0;
}



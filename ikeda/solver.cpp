#include "bits/stdc++.h"

#include "./ikeda.hpp"

int main(int argc, char *argv[]) 
{
    if (argc != 2) return 1;
    string path(argv[1]);
    int x, y;
    auto board = boardloader::load_board(path, x, y);
    auto ret = ikeda::largest_rectangle(board);
    cout << "fin" << endl;
    auto graph = ikeda::get_graph(board, ret);
    for (int i = 0; i < graph.size(); i++) {
        for (int j = 0; j < graph[i].size(); j++) {
            cout << graph[i][j] << " ";
        }
        cout << endl;
    }
    return 0;
}



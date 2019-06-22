#include "bits/stdc++.h"

#include "./ikeda.hpp"


int main(int argc, char *argv[]) 
{
    int h; cin >> h;
    vector<string> board(h);
    for (int i = 0; i < h; i++) {
        cin >> board[i];
    }
    vector<vector<pair<int, int>>> ret = ikeda::get_tenretsu(board);

    cout << ret.size() << endl;
    for (vector<pair<int, int>> i : ret) {
        cout << "points : " << endl;
        for (pair<int, int> j : i) {
            cout << j.first << " " << j.second << endl;
        }
    }

    /*
    string path(argv[1]);
    int x, y;
    auto board = boardloader::load_board(path, x, y);
    auto blocks = ikeda::largest_rectangle(board);
    ikeda::init_board(board.size(), board[0].size());

    vector<int> used(blocks.size(), 0);
    boardloader::Point p(x, y);
    int dir = 1; // 0123 -> urdl

    for (int _ = 0; _ < blocks.size(); _++) {
        int score = 1010001000, targ = -1;
        for (int i = 0; i < used.size(); i++) {
            if (!used[i]) {
                int tmp = ikeda::calc_distance(board, p, blocks[i].small);
                if (score > tmp) {
                    score = tmp;
                    targ = i;
                }
            }
        }
        used[targ] = 1;
        auto tmp = ikeda::move(board, p, blocks[targ].small);
        //cout << " " << p << " " << blocks[targ].small << endl;
        //cout << tmp << endl;
        ikeda::paint_string(p, dir, tmp);
        //cout << " " << p << " " << blocks[targ].small << endl;
        //ikeda::debug_print();
        std::cout << tmp;
        ikeda::paint(board, blocks[targ], p, dir);
    }
    cout << endl;
    */
    return 0;
}



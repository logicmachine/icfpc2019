#pragma once

#include "../sayama/board_loader.hpp"

using namespace std;

namespace ikeda
{
    template<class T> bool chmax(T &a, const T &b) { if (a < b) { a = b; return true; } return false; }
    template<class T> bool chmin(T &a, const T &b) { if (b < a) { a = b; return true; } return false; }

    using Weight = int;
    using Flow = int;
    struct Edge {
        int s, d; Weight w; Flow c;
        Edge() {};
        Edge(int s, int d, Weight w = 1) : s(s), d(d), w(w), c(w) {};
    };
    bool operator<(const Edge &e1, const Edge &e2) { return e1.w < e2.w; }
    bool operator>(const Edge &e1, const Edge &e2) { return e2 < e1; }
    inline ostream &operator<<(ostream &os, const Edge &e) { return (os << '(' << e.s << ", " << e.d << ", " << e.w << ')'); }

    using Edges = vector<Edge>;
    using Graph = vector<Edges>;
    using Array = vector<Weight>;
    using Matrix = vector<Array>;

    void addArc(Graph &g, int s, int d, Weight w = 1) {
        g[s].emplace_back(s, d, w);
    }
    void addEdge(Graph &g, int a, int b, Weight w = 1) {
        addArc(g, a, b, w);
        addArc(g, b, a, w);
    }

    typedef pair<int, int> point;
    struct Block {
        point small, large, centor;
        int size, index;
        Block(const Block &a):
            small(a.small), large(a.large), centor(a.centor), size(a.size), index(a.index)
        {}
        Block(const point &st, const point &en) : small(st), large(en)
        {
            size = (en.first - st.first + 1) * (en.second - st.second + 1);
        }
    };
    inline ostream &operator<<(ostream &os, const Block &e) { return (os << '(' << e.small.first << ", " << e.small.second << ")->(" 
            << e.large.first << ',' << e.large.second << "), " << e.size << ' '); }
    Block BLOCK_ZERO(point(0, 0), point(-1, -1));

    Block _largest_rectangle(vector<vector<int>> &board) {
        int H = board.size(), W = board[0].size();
        vector<vector<int>> v(H, vector<int>(W));
        bool fill = true;
        for (int i = 0; i < H; i++) {
            for (int j = 0; j < W; j++) {
                v[i][j] = board[i][j];
                fill &= !v[i][j]; // 空マスを 0 にして & をとる
            }
        }
        if (fill) return BLOCK_ZERO;
        // 下向き累積和
        for (int i = 0; i < H-1; i++) {
            for (int j = 0; j < W; j++) {
                if (v[i + 1][j] == 0)continue;
                v[i + 1][j] += v[i][j];
            }
        }
        using P = pair<int, int>;
        using state = pair<P, P>;
        int ans = 0;
        state ans_state;
        for (int i = 0; i < H; i++) {
            stack<P> st;
            st.emplace(0, 0);
            for (int j = 0; j < W; j++) {
                if (st.top().first == v[i][j])
                    continue;
                int pos = j;
                while (st.top().first > v[i][j]) {
                    if (ans < st.top().first * (j - st.top().second)) {
                        ans = st.top().first * (j - st.top().second);
                        ans_state = make_pair(make_pair(i - st.top().first + 1, st.top().second), make_pair(i, j-1));
                    }
                    pos = st.top().second;
                    st.pop();
                }
                st.emplace(v[i][j], pos);
            }
            while (st.size()) {
                if (ans < st.top().first*(W - st.top().second)) {
                    ans = st.top().first*(W - st.top().second);
                    ans_state = make_pair(make_pair(i - st.top().first + 1, st.top().second), make_pair(i, W-1));
                }
                st.pop();
            }
        }
        return Block(ans_state.first, ans_state.second);
    }

    void fill_state(vector<vector<int>> &state, Block &b)
    {
        for (int i = b.small.first; i <= b.large.first; i++) {
            for (int j = b.small.second; j <= b.large.second; j++) {
                state[i][j] = 0;
            }
        }
    }

    vector<Block> largest_rectangle(vector<vector<boardloader::Cell>> &board)
    {
        vector<vector<int>> state(board.size(), vector<int>(board[0].size(), 0));
        for (int i = 0; i < board.size(); i++) {
            for (int j = 0; j < board[i].size(); j++) {
                state[i][j] = (board[i][j] != boardloader::Cell::Obstacle); // 空マスが 1 
            }
        }
        vector<Block> ret;
        auto block = _largest_rectangle(state);
        while (block.size) {
            ret.emplace_back(block);
            fill_state(state, block);
            block = _largest_rectangle(state);
        }
        return ret;
    }

    vector<vector<int>> get_graph(vector<Block> &blocks)
    {
        vector<vector<int>> graph(blocks.size(), vector<int>(blocks.size(), 0));
        for (int i = 0; i < blocks.size(); i++) {
            for (int j = i+1; j < blocks.size(); j++) {
                auto check = [&](Block &a, Block &b) {
                    bool ret = false;
                    ret |= (abs(a.small.first - b.small.first) == 1);
                    ret |= (abs(a.small.first - b.small.second) == 1);
                    ret |= (abs(a.small.first - b.large.first) == 1);
                    ret |= (abs(a.small.first - b.large.second) == 1);
                    ret |= (abs(a.small.second - b.small.first) == 1);
                    ret |= (abs(a.small.second - b.small.second) == 1);
                    ret |= (abs(a.small.second - b.large.first) == 1);
                    ret |= (abs(a.small.second - b.large.second) == 1);
                    ret |= (abs(a.large.first - b.small.first) == 1);
                    ret |= (abs(a.large.first - b.small.second) == 1);
                    ret |= (abs(a.large.first - b.large.first) == 1);
                    ret |= (abs(a.large.first - b.large.second) == 1);
                    ret |= (abs(a.large.second - b.small.first) == 1);
                    ret |= (abs(a.large.second - b.small.second) == 1);
                    ret |= (abs(a.large.second - b.large.first) == 1);
                    ret |= (abs(a.large.second - b.large.second) == 1);
                    return ret;
                };
                if (check(blocks[i], blocks[j])) {
                    graph[i][j] = 1;
                    graph[j][i] = 1;
                }
            }
        }
        return graph;
    }

}

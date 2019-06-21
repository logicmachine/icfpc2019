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

    typedef boardloader::Point point;
    struct Block {
        point small, large, centor;
        int size, index;
        Block(const Block &a):
            small(a.small), large(a.large), centor(a.centor), size(a.size), index(a.index)
        {}
        Block(const point &st, const point &en) : small(st), large(en)
        {
            size = (en.x - st.x + 1) * (en.y - st.y + 1);
        }
    };
    inline ostream &operator<<(ostream &os, const Block &e) { return (os << '(' << e.small.y << ", " << e.small.x << ")->(" 
            << e.large.y << ',' << e.large.x << "), " << e.size << ' '); }
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
        using state = pair<point, point>;
        using P = pair<int, int>;
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
                        ans_state = make_pair(point(i - st.top().first + 1, st.top().second), point(i, j-1));
                    }
                    pos = st.top().second;
                    st.pop();
                }
                st.emplace(v[i][j], pos);
            }
            while (st.size()) {
                if (ans < st.top().first*(W - st.top().second)) {
                    ans = st.top().first*(W - st.top().second);
                    ans_state = make_pair(point(i - st.top().first + 1, st.top().second), point(i, W-1));
                }
                st.pop();
            }
        }
        return Block(ans_state.first, ans_state.second);
    }

    void fill_state(vector<vector<int>> &state, Block &b)
    {
        for (int i = b.small.y; i <= b.large.y; i++) {
            for (int j = b.small.x; j <= b.large.x; j++) {
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

    int dxy[5] = {0, 1, 0, -1, 0};
    int calc_distance(vector<vector<boardloader::Cell>> &board, boardloader::Point &from, boardloader::Point &to)
    {
        vector<vector<int>> cost(board.size(), vector<int>(board[0].size(), 1010001000));
        cost[from.y][from.x] = 0;
        deque<pair<int, boardloader::Point>> que;
        que.push_back({0, from});
        while (!que.empty()) {
            auto pos = que.front().second;
            auto cs = que.front().first;
            que.pop_front();
            if (cs != cost[pos.y][pos.x]) continue;
            for (int i = 0; i < 4; i++) {
                int nx = pos.y + dxy[i], ny = pos.x + dxy[i+1];
                if (0 <= nx && nx < board.size() &&
                        0 <= ny && ny < board[nx].size() &&
                        board[nx][ny] != boardloader::Cell::Obstacle && 
                        cost[nx][ny] > cs+1) {
                    cost[nx][ny] = cs+1;
                    que.push_back({cs+1, boardloader::Point(nx, ny)});
                }
            }
        }
        return cost[to.y][to.x];
    }

    vector<vector<int>> get_graph(vector<vector<boardloader::Cell>> &board, vector<Block> &blocks)
    {
        vector<vector<int>> graph(blocks.size(), vector<int>(blocks.size(), 0));
        for (int i = 0; i < blocks.size(); i++) {
            for (int j = i+1; j < blocks.size(); j++) {
                auto check = [&](Block &a, Block &b) {
                    bool ret = false;
                    ret |= (abs(a.small.y - b.small.y) == 1);
                    ret |= (abs(a.small.y - b.small.x) == 1);
                    ret |= (abs(a.small.y - b.large.y) == 1);
                    ret |= (abs(a.small.y - b.large.x) == 1);
                    ret |= (abs(a.small.x - b.small.y) == 1);
                    ret |= (abs(a.small.x - b.small.x) == 1);
                    ret |= (abs(a.small.x - b.large.y) == 1);
                    ret |= (abs(a.small.x - b.large.x) == 1);
                    ret |= (abs(a.large.y - b.small.y) == 1);
                    ret |= (abs(a.large.y - b.small.x) == 1);
                    ret |= (abs(a.large.y - b.large.y) == 1);
                    ret |= (abs(a.large.y - b.large.x) == 1);
                    ret |= (abs(a.large.x - b.small.y) == 1);
                    ret |= (abs(a.large.x - b.small.x) == 1);
                    ret |= (abs(a.large.x - b.large.y) == 1);
                    ret |= (abs(a.large.x - b.large.x) == 1);
                    return ret;
                };
                if (check(blocks[i], blocks[j])) {
                    int tmp = calc_distance(board, blocks[i].small, blocks[j].small);
                    graph[i][j] = tmp;
                    graph[j][i] = tmp;
                }
            }
        }
        return graph;
    }


}


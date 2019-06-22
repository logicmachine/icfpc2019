#pragma once

#include "../sayama/board_loader.hpp"

using namespace std;

namespace ikeda
{
    template<class T> bool chmax(T &a, const T &b) { if (a < b) { a = b; return true; } return false; }
    template<class T> bool chmin(T &a, const T &b) { if (b < a) { a = b; return true; } return false; }

    vector<vector<int>> painted;
    int init_fl = 0;

    void debug_print()
    {
        for (int i = 0; i < painted.size(); i++) {
            for (int j = 0; j < painted[0].size(); j++) {
                cout << painted[i][j] << " ";
            }
            cout << endl;
        }
    }

    void init_board(int h, int w)
    {
        painted.assign(h, vector<int>(w, 0));
        init_fl = 1;
    }

    void paint_pos(int y, int x)
    {
        painted[y][x] = 1;
        /*
        if (init_fl && 0 <= y && y < painted.size()
                && 0 <= x && x < painted[0].size()) painted[y][x] = 1;
                */
    }

    int dirdist[4][3][2] {
        {{1, 0}, {1, -1}, {1, 1},},
        {{0, 1}, {1, 1}, {-1, 1},},
        {{-1, 0}, {-1, -1}, {-1, 1},},
        {{0, -1}, {1, -1}, {-1, -1},},
    };
    void paint_pos(boardloader::Point &p, int dir)
    {
        paint_pos(p.y, p.x);
        for (int i = 0; i < 3; i++) {
            int ny = p.y + dirdist[dir][i][0], nx = p.x + dirdist[dir][i][1];
            if (0 <= ny && ny < painted.size() &&
                0 <= nx && nx < painted[0].size()) paint_pos(ny, nx);
        }
    }

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

    vector<Block> largest_rectangle(vector<string> &board)
    {
        vector<vector<int>> state(board.size(), vector<int>(board[0].size(), 0));
        for (int i = 0; i < board.size(); i++) {
            for (int j = 0; j < board[i].size(); j++) {
                state[i][j] = (board[i][j] != '#'); // 空マスが 1 
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
    int calc_distance(vector<vector<boardloader::Cell>> &board, const boardloader::Point &from, const boardloader::Point &to)
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

    int calc_distance(vector<string> &board, const boardloader::Point &from, const boardloader::Point &to)
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
                        board[nx][ny] != '#' && 
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

    vector<vector<int>> get_graph(vector<string> &board, vector<Block> &blocks)
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

    //int dxy[5] = {0, 1, 0, -1, 0};
    string cmdchar[4] = {"A", "S", "D", "W"};
    string move(vector<vector<boardloader::Cell>> &board, const boardloader::Point &from, const boardloader::Point &to)
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
        string cmd;
        auto p = to;
        while (p != from) {
            for (int i = 0; i < 4; i++) {
                int nx = p.y + dxy[i], ny = p.x + dxy[i+1];
                if (0 <= nx && nx < board.size() &&
                        0 <= ny && ny < board[nx].size() &&
                        cost[nx][ny] == cost[p.y][p.x]-1) {
                    cmd += cmdchar[i];
                    p = boardloader::Point(nx, ny);
                    break;
                }
            }
        }
        reverse(cmd.begin(), cmd.end());
        return cmd;
    }

    void dir_to_right(int &dir)
    {
        if (dir == 2) {
            dir = (dir+3) % 4;
            cout << "Q";
        }
        while (dir != 1) {
            dir = (dir+1) % 4;
            cout << "E";
        }
    }

    void dir_to_right(boardloader::Point &p, int &dir)
    {
        if (dir == 2) {
            dir = (dir+3) % 4;
            cout << "Q";
        }
        while (dir != 1) {
            dir = (dir+1) % 4;
            cout << "E";
        }
        paint_pos(p, dir);
    }

    void dir_to_down(int &dir)
    {
        if (dir == 3) {
            dir = (dir+3) % 4;
            cout << "Q";
        }
        while (dir != 2) {
            dir = (dir+1) % 4;
            cout << "E";
        }
    }

    void dir_to_down(boardloader::Point &p, int &dir)
    {
        if (dir == 3) {
            dir = (dir+3) % 4;
            cout << "Q";
        }
        while (dir != 2) {
            dir = (dir+1) % 4;
            cout << "E";
        }
        paint_pos(p, dir);
    }

    void dir_to_up(int &dir)
    {
        if (dir == 1) {
            dir = (dir+3) % 4;
            cout << "Q";
        }
        while (dir != 0) {
            dir = (dir+1) % 4;
            cout << "E";
        }
    }

    void dir_to_up(boardloader::Point &p, int &dir)
    {
        if (dir == 1) {
            dir = (dir+3) % 4;
            cout << "Q";
        }
        while (dir != 0) {
            dir = (dir+1) % 4;
            cout << "E";
        }
        paint_pos(p, dir);
    }

    void move_right(boardloader::Point &p)
    {
        cout << "D";
        p.x++;
    }

    void move_right(boardloader::Point &p, int dir)
    {
        move_right(p);
        paint_pos(p, dir);
    }

    void move_left(boardloader::Point &p)
    {
        cout << "A";
        p.x--;
    }

    void move_left(boardloader::Point &p, int dir)
    {
        move_left(p);
        paint_pos(p, dir);
    }

    void move_down(boardloader::Point &p)
    {
        cout << "S";
        p.y--;
    }

    void move_down(boardloader::Point &p, int dir)
    {
        move_down(p);
        paint_pos(p, dir);
    }

    void move_up(boardloader::Point &p)
    {
        cout << "W";
        p.y++;
    }

    void move_up(boardloader::Point &p, int dir)
    {
        move_up(p);
        paint_pos(p, dir);
    }

    void paint_string(boardloader::Point &st, int dir, const string &cmd)
    {
        for (int i = 0; i < cmd.size(); i++) {
            if (cmd[i] == 'W') {
                st.y++;
                paint_pos(st, dir);
            } else if (cmd[i] == 'D') {
                st.x++;
                paint_pos(st, dir);
            } else if (cmd[i] == 'S') {
                st.y--;
                paint_pos(st, dir);
            } else if (cmd[i] == 'A') {
                st.x--;
                paint_pos(st, dir);
            }
        }
    }

    bool filled(Block &b)
    {
        if (init_fl) {
            for (int i = b.small.y; i <= b.large.y; i++) {
                for (int j = b.small.x; j <= b.large.x; j++) {
                    if (!painted[i][j]) return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }

    void paint(vector<vector<boardloader::Cell>> &board, Block &block, boardloader::Point &p, int &dir)
    {
        if (filled(block)) {
            //cout << "filled!" << endl;
            return;
        } else {
            //cout << "block : " << block.small << " " << block.large << endl;
        }
        dir_to_right(p, dir);
        bool fl = false;
        if ((block.large.x - block.small.x + 1) > 1) move_right(p, dir);
        for (int j = 0; j < (block.large.x - block.small.x + 1) / 6; j++) {
            fl = true;
            if (j) {
                move_right(p, dir);
                move_right(p, dir);
                move_right(p, dir);
            }
            dir_to_up(p, dir);
            for (int i = block.small.y; i < block.large.y; i++) {
                move_up(p, dir);
            }
            dir_to_right(p, dir);
            move_right(p, dir);
            move_right(p, dir);
            move_right(p, dir);
            dir_to_down(p, dir);
            for (int i = block.small.y; i < block.large.y; i++) {
                move_down(p, dir);
            }
            dir_to_right(p, dir);
        }
        if ((block.large.x - block.small.x + 1) % 6 >= 4) {
            if (fl) {
                move_right(p, dir);
                move_right(p, dir);
                move_right(p, dir);
            }
            fl = true;
            dir_to_up(p, dir);
            for (int i = block.small.y; i < block.large.y; i++) {
                move_up(p, dir);
            }
            dir_to_right(p, dir);
            for (int i = 0; i < (block.large.x - block.small.x + 1) % 3; i++) {
                move_right(p, dir);
            }
            dir_to_down(p, dir);
            for (int i = block.small.y; i < block.large.y; i++) {
                move_down(p, dir);
            }
            dir_to_right(p, dir);
        } else if ((block.large.x - block.small.x + 1) % 6 >= 1) {
            if (fl) {
                for (int i = 0; i < (block.large.x - block.small.x + 1) % 3 + 1; i++) {
                    move_right(p, dir);
                }
            }
            fl = true;
            //cout << "dir : " << dir << endl;
            dir_to_up(p, dir);
            for (int i = block.small.y; i < block.large.y; i++) {
                move_up(p, dir);
            }
        }
    }
}


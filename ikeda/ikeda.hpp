#pragma once

#include "../sayama/board_loader.hpp"
#include <stack>
#include <deque>
#include <set>
#include <string>

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

    string dir_to_right(int &dir)
    {
        if (dir == 2) {
            dir = 1;
            return "Q";
        }
        string ret;
        while (dir != 1) {
            dir = (dir+1) % 4;
            ret += "E";
        }
        return ret;
    }

    string dir_to_down(int &dir)
    {
        if (dir == 3) {
            dir = 2;
            return "Q";
        }
        string ret;
        while (dir != 2) {
            dir = (dir+1) % 4;
            ret += "E";
        }
        return ret;
    }

    string dir_to_up(int &dir)
    {
        if (dir == 1) {
            dir = 0;
            return "Q";
        }
        string ret;
        while (dir != 0) {
            dir = (dir+1) % 4;
            ret += "E";
        }
        return ret;
    }

    string move_right(boardloader::Point &p)
    {
        p.x++;
        return "D";
    }

    string move_left(boardloader::Point &p)
    {
        p.x--;
        return "A";
    }

    string move_down(boardloader::Point &p)
    {
        p.y--;
        return "S";
    }
    
    string move_up(boardloader::Point &p)
    {
        p.y++;
        return "W";
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

    string paint(vector<vector<boardloader::Cell>> &board, Block &block, boardloader::Point &p, int &dir)
    {
        string ret;
        bool fl = false;
        if ((block.large.x - block.small.x + 1) > 1) {
            ret += move_right(p);
        }
        for (int j = 0; j < (block.large.x - block.small.x + 1) / 6; j++) {
            fl = true;
            if (j) {
                ret += move_right(p);
                ret += move_right(p);
                ret += move_right(p);
            }
            ret += dir_to_up(dir);
            for (int i = block.small.y; i < block.large.y; i++) {
                ret += move_up(p);
            }
            ret += dir_to_right(dir);
            ret += move_right(p);
            ret += move_right(p);
            ret += move_right(p);
            ret += dir_to_down(dir);
            for (int i = block.small.y; i < block.large.y; i++) {
                ret += move_down(p);
            }
            ret += dir_to_right(dir);
        }
        if ((block.large.x - block.small.x + 1) % 6 != 0) {
            if (fl) {
                ret += move_right(p);
                ret += move_right(p);
                if ((block.large.x - block.small.x + 1) % 6 != 1) {
                    ret += move_right(p);
                }
            }
            fl = true;
            ret += dir_to_up(dir);
            for (int i = block.small.y; i < block.large.y; i++) {
                ret += move_up(p);
            }
            ret += dir_to_right(dir);
        }
        if ((block.large.x - block.small.x + 1) % 6 >= 4) {
            ret += move_right(p);
            ret += move_right(p);
            ret += dir_to_down(dir);
            for (int i = block.small.y; i < block.large.y; i++) {
                ret += move_down(p);
            }
        }
        return ret;
    }

    void get_edge(vector<string> &data, vector<vector<int>> &tate, vector<vector<int>> &yoko)
    {
        for (int i = 0; i < data.size(); i++) {
            for (int j = 0; j < data[i].size()+1; j++) {
                int lp = 0, rp = 0;
                if (!j || data[i][j-1] == '#') lp = 1;
                if (j == data[0].size() || data[i][j] == '#') rp = 1;
                if (lp != rp) tate[i][j] = 1;
            }
        }
        for (int i = 0; i < data.size()+1; i++) {
            for (int j = 0; j < data[0].size(); j++) {
                int lp = 0, rp = 0;
                if (!i || data[i-1][j] == '#') lp = 1;
                if (i == data.size() || data[i][j] == '#') rp = 1;
                if (lp != rp) yoko[i][j] = 1;
            }
        }
    }

    int yokoy[4] = {0, 0, 1, 1}, yokox[4] = {0, -1, 0, -1},
        tatey[4] = {-1, 0, -1, 0}, tatex[4] = {0, 0, 1, 1};
    vector<pair<int, int>> generate_point(vector<vector<int>> &tate, vector<vector<int>> &yoko, int y, int x)
    {
        vector<pair<int, int>> ret;
        int sty = y, stx = x, by = -5, bx = -5;
        bool tateyoko = true;
        do {
            if (tateyoko) {
                if (by != y+1 && 0 <= y+1 && y+1 < tate.size() && tate[y+1][x]) {
                    tate[y+1][x] = 0;
                    by = y;
                    y++;
                } else if (by != y-1 && 0 <= y-1 && y-1 < tate.size() && tate[y-1][x]) {
                    tate[y-1][x] = 0;
                    by = y;
                    y--;
                } else {
                    for (int i = 0; i < 4; i++) {
                        int ny = y + yokoy[i], nx = x + yokox[i];
                        if (by == ny && bx == nx) continue;
                        if (0 <= ny && ny < yoko.size() && 
                                0 <= nx && nx < yoko[0].size() &&
                                yoko[ny][nx]) {
                            ret.push_back({y+i/2, x});
                            yoko[ny][nx] = 0;
                            by = y; bx = x;
                            y = ny; x = nx;
                            tateyoko = !tateyoko;
                            break;
                        }
                    }
                }
            } else {
                if (bx != x+1 && 0 <= x+1 && x+1 < yoko.size() && yoko[y][x+1]) {
                    yoko[y][x+1] = 0;
                    bx = x;
                    x++;
                } else if (bx != x-1 && 0 <= x-1 && x-1 < yoko.size() && yoko[y][x-1]) {
                    yoko[y][x-1] = 0;
                    bx = x;
                    x--;
                } else {
                    for (int i = 0; i < 4; i++) {
                        int ny = y + tatey[i], nx = x + tatex[i];
                        if (by == ny && bx == nx) continue;
                        if (0 <= ny && ny < tate.size() && 
                                0 <= nx && nx < tate[0].size() &&
                                tate[ny][nx]) {
                            ret.push_back({y, x+i/2});
                            tate[ny][nx] = 0;
                            by = y; bx = x;
                            y = ny; x = nx;
                            tateyoko = !tateyoko;
                            break;
                        }
                    }
                }
            }
        } while (sty != y || stx != x || !tateyoko);
        tate[sty][stx] = 0;
        return ret;
    }

    inline double cross(const pair<int, int> &a, const pair<int, int> &b){
        return a.first * b.second - a.second * b.first;
    }

    bool is_clockwize(vector<pair<int, int>> &data)
    {
		double s = 0.0;
		for(int i = 0; i < data.size(); ++i){
			s += cross(data[i], data[(i + 1) % data.size()]);
		}
		return s <= 0.0;
    }

    vector<vector<pair<int, int>>> generate_points(vector<vector<int>> &tate, vector<vector<int>> &yoko)
    {
        vector<vector<pair<int, int>>> ret;
        for (int i = 0; i < tate.size(); i++) {
            for (int j = 0; j < tate[i].size(); j++) {
                if (tate[i][j]) {
                    auto tmp = generate_point(tate, yoko, i, j);
                    if (!is_clockwize(tmp)) {
                        reverse(tmp.begin(), tmp.end());
                    }
                    ret.push_back(tmp);
                }
            }
        }
        return ret;
    }

    vector<vector<pair<int, int>>> get_tenretsu(vector<string> &data)
    {
        vector<vector<int>> tate(data.size(), vector<int>(data[0].size()+1, 0)), 
            yoko(data.size()+1, vector<int>(data[0].size(), 0));
        get_edge(data, tate, yoko);
        return generate_points(tate, yoko);
    }

    void connect(vector<string> &board, set<pair<int, int>> ng, pair<int, int> pos)
    {
        vector<vector<int>> len(board.size(), vector<int>(board[0].size(), 1010001000));
        for (int i = 0; i < board.size(); i++) {
            for (int j = 0; j < board[0].size(); j++) {
                if (ng.find({i, j}) != ng.end()) len[i][j] = -1;
            }
        }
        deque<pair<int, pair<int, int>>> q;
        len[pos.first][pos.second] = 0;
        q.push_front({0, pos});
        int finx = -100, finy = -100;
        while (!q.empty()) {
            auto pp = q.front(); q.pop_front();
            if (pp.first != len[pp.second.first][pp.second.second]) continue;
            for (int i = 0; i < 4; i++) {
                int ny = pp.second.first + dxy[i], nx = pp.second.second + dxy[i+1];
                if (0 <= ny && ny < board.size() && 
                        0 <= nx && nx < board[0].size() && 
                        len[ny][nx] != -1 && 
                        len[ny][nx] > len[pp.second.first][pp.second.second] + 1) {
                    len[ny][nx] = len[pp.second.first][pp.second.second] + 1;
                    q.push_back({len[ny][nx], {ny, nx}});
                    if (board[ny][nx] == '#' && 
                            (finx == -100 || len[finy][finx] > len[ny][nx])) {
                        finy = ny; finx = nx;
                    }
                }
            }
        }
        while (finy != pos.first || finx != pos.second) {
            board[finy][finx] = '#';
            for (int i = 0; i < 4; i++) {
                int ny = finy + dxy[i], nx = finx + dxy[i+1];
                if (0 <= ny && ny < board.size() && 
                        0 <= nx && nx < board[0].size() && 
                        len[ny][nx] == len[finy][finx]-1) {
                    finy = ny; finx = nx;
                    break;
                }
            }
        }
        board[pos.first][pos.second] = '#';
    }
}


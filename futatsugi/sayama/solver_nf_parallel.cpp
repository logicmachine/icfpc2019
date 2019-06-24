#include "../../sayama/board_loader.hpp"

#include "bits/stdc++.h"

#include <algorithm>
#include <queue>
#include <random>
#include <stack>
#include <tuple>

#include "tbb/task_scheduler_init.h"
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#include "tbb/parallel_reduce.h"
#include "tbb/parallel_sort.h"

#include "../ccl.hpp"
#include "../../ikeda/ikeda.hpp"

using boardloader::Cell;
using boardloader::load_board;
using boardloader::Point;
using boardloader::print_table;
using boardloader::Table;

namespace ccl {

nf_ccl::CCL ccl;
std::vector<int> data;
std::vector<int> spaces;

void get_ccl_data(const std::vector<std::vector<Cell>>& table, std::vector<int>& data, int& W)
{
    W = table[0].size();
    //vector<int> data;
    for (const auto& xs : table) {
        for (const auto& x : xs) {
            data.push_back((x == Cell::Obstacle || x == Cell::Occupied) ? 1 : 0);
        }
    }
}

void calc_ccl(std::vector<int>& data, const int W, std::vector<int>& result, vector<int>& spaces)
{
	//std::vector<int> result(ccl.ccl(data, W));
    result = ccl.ccl(data, W);

    //std::vector<int> memo;
	for (int i = 0; i < static_cast<int>(result.size()) / W; i++) {
		for (int j = 0; j < W; j++) {
            if (data[i*W+j] == 0 && find(spaces.begin(), spaces.end(), result[i*W+j]) == spaces.end()) {
                spaces.push_back(result[i*W+j]);
            }
        }
	}
}

void check_ccl(const std::vector<std::vector<Cell>>& table, vector<int>& result, vector<int>& spaces)
{
    int W;
    vector<int> data;
    result.clear();
    spaces.clear();
    get_ccl_data(table, data, W);
    calc_ccl(data, W, result, spaces);

	//std::cerr << "Size: " << result.size() << std::endl; /// number of pixels
	//std::cerr << "Width: " << W << std::endl; /// width

#if 0
    std::cerr << "conncted components: ";
    for (auto space : spaces) {
        std::cerr << space << " ";
    }
    std::cerr << std::endl;
#endif
    //std::cerr << "number of conncted components: " << spaces.size() << std::endl;
}

} // namespace ccl

namespace xyzworker
{

class Cluster
{
public:
    Table<int> clustering_by_bfs(Table<Cell>& board, std::vector<Point>& point_list);
    std::vector<int> cluster_count_by_bfs(Table<Cell>& board, std::vector<Point>& point_list);
};

std::vector<int> Cluster::cluster_count_by_bfs(Table<Cell>& board, std::vector<Point>& point_list)
{
    auto table = clustering_by_bfs(board, point_list);

    std::vector<int> result(point_list.size(), 0);

    for (int i = 0; i < board.size(); i++)
    {
        for (int j = 0; j < board[i].size(); j++)
        {
            if (board[i][j] != Cell::Obstacle)
            {
                result[table[i][j]]++;
            }
        }
    }
    return result;
}

Table<int> Cluster::clustering_by_bfs(Table<Cell>& board, std::vector<Point>& point_list)
{
    std::array<int, 4> dx = { 0, 1, 0, -1 };
    std::array<int, 4> dy = { 1, 0, -1, 0 };

    const int height = board.size();
    const int width = board[0].size();

    constexpr int NONE = -1;

    Table<int> ret(height, std::vector<int>(width, NONE));

    std::queue<std::pair<int, Point>> que;
    for (int i = 0; i < point_list.size(); i++)
    {
        que.emplace(i, point_list[i]);
        ret[point_list[i].y][point_list[i].x] = i;
    }

    while (!que.empty())
    {
        int id;
        Point p;
        std::tie(id, p) = que.front();
        que.pop();

        for (int i = 0; i < 4; i++)
        {
            const int ny = p.y + dy[i];
            const int nx = p.x + dx[i];

            if (0 <= ny && ny < height && 0 <= nx && nx < width && board[ny][nx] != Cell::Obstacle && ret[ny][nx] == NONE)
            {
                ret[ny][nx] = id;
                que.emplace(id, Point(ny, nx));
            }
        }
    }
    return ret;
}

class HCPSolver
{
public:
    // start は 0番目とする
    static std::vector<int> solve(const Table<int>& distance)
    {
        if (distance.size() == 1)
        {
            std::vector<int> ret = { 0 };
            return ret;
        }

        constexpr int INF = std::numeric_limits<int>::max() / 100;

        const int dim = distance.size();

        Table<int> rec(dim, std::vector<int>(1 << dim, INF));
        Table<int> dp(dim, std::vector<int>(1 << dim, INF));

        dp[0][1] = 0;

        const int all = (1 << dim) - 1;

        for (int S = 1; S <= all; S++)
        {
            for (int s = 0; s < dim; s++)
            {
                if ((S & (1 << s)) != 0)
                {
                    for (int e = 0; e < dim; e++)
                    {
                        if (distance[s][e] != INF && (~S & (1 << e)) != 0)
                        {
                            const int nS = S | (1 << e);
                            if (dp[e][nS] > dp[s][S] + distance[s][e])
                            {
                                dp[e][nS] = dp[s][S] + distance[s][e];
                                rec[e][nS] = s;
                            }
                        }
                    }
                }
            }
        }
        int best_end = 0;
        int ans = INF;
        for (int i = 0; i < dim; i++)
        {
            if (ans > dp[i][all])
            {
                ans = dp[i][all];
                best_end = i;
            }
        }
        std::vector<int> tour;
        int city = best_end;
        int S = all;
        for (int i = 0; i < dim; i++)
        {
            const int next = rec[city][S];
            S &= ~(1 << city);
            tour.push_back(city);
            city = next;
        }
        std::reverse(tour.begin(), tour.end());

        assert(tour.size() == dim);
        return tour;
    }
};

enum class Direction
{
    Up,
    Left,
    Down,
    Right,
    None,
};

std::string direction_to_string(const Direction dir)
{
    switch (dir)
    {
    case Direction::Up:
        return "W";
    case Direction::Left:
        return "A";
    case Direction::Down:
        return "S";
    case Direction::Right:
        return "D";
    }
}

Direction clockwise(const Direction dir)
{
    switch (dir)
    {
    case Direction::Up:
        return Direction::Right;
    case Direction::Right:
        return Direction::Down;
    case Direction::Down:
        return Direction::Left;
    case Direction::Left:
        return Direction::Up;
    case Direction::None:
        assert(false);
    }
}

Direction counter_clockwise(const Direction dir)
{
    switch (dir)
    {
    case Direction::Up:
        return Direction::Left;
    case Direction::Left:
        return Direction::Down;
    case Direction::Down:
        return Direction::Right;
    case Direction::Right:
        return Direction::Up;
    case Direction::None:
        assert(false);
    }
}

Direction inverse(const Direction dir)
{
    switch (dir)
    {
    case Direction::Up:
        return Direction::Down;
    case Direction::Left:
        return Direction::Right;
    case Direction::Down:
        return Direction::Up;
    case Direction::Right:
        return Direction::Left;
    case Direction::None:
        assert(false);
    }
}

enum class ActionType
{
    Move,
    DoNothing,
    TurnClockwise,
    TurnCounterClockwise,
    Attatch,
    FastWheels,
    Drill,
    Cloning,
    Teleport,
};

class Action
{
    ActionType type;
    Point relative_position;
    Direction direction;

public:
    Action(ActionType type, Point relative_position)
        : type(type)
        , relative_position(relative_position)
        , direction(Direction::None)
    {
    }

    Action(ActionType type, Direction direction)
        : type(type)
        , relative_position()
        , direction(direction)
    {
    }

    Action(ActionType type)
        : type(type)
        , relative_position()
        , direction(Direction::None)
    {
    }

    Action()
        : type(ActionType::DoNothing)
        , relative_position()
        , direction(Direction::None)
    {
    }

    std::string to_string()
    {
        switch (type)
        {
        case ActionType::Move:
            return direction_to_string(direction);
        case ActionType::TurnClockwise:
            return "E";
        case ActionType::TurnCounterClockwise:
            return "Q";
        case ActionType::Attatch:
            return "B" + relative_position.to_string_output();
        case ActionType::FastWheels:
            return "F";
        case ActionType::Drill:
            return "L";
        case ActionType::Cloning:
            return "C";
        case ActionType::Teleport:
            return "R";
        }
    }
};

class Worker
{
public:
    Worker(Table<Cell>& table, int y, int x);
    Worker(Table<Cell>& table, int y, int x, int idx);
    std::vector<Action> solve();

    void init(Table<Cell>& table, int y, int x);

    Worker();
    Worker(const Worker& src);
    void set_index(const int idx_) { idx = idx_; }
    void copy_from(const Worker& src);

    Table<Cell>& get_table();

    std::vector<Action>& get_action_list();

    std::vector<Point> gather(Cell cell);

    template <typename FUNCTION>
    std::vector<Point> gather_with(FUNCTION condition);

    template <typename FUNCTION>
    bool bfs_move(FUNCTION condition);

    template <typename FUNCTION>
    int shortest_distance_to(const Point& start, FUNCTION condition);

    void dfs_with_restart();

    void collect_optimal(std::vector<Point>& target_list);

    int select_shortest(std::vector<Point>& target_list);

    // Command

    void move(Direction dir);

    void clone_command();

    void attach_command();

    /// nf
    Direction get_move_direction(char a);
    Point nearest_cc(int y, int x);
    void search(Table<Cell>& search_table, int y, int x, int score, std::vector<Direction>& path);
    void search_depth(Table<Cell>& search_table, int y, int x, int score, std::vector<Direction>& path);
    int wrap_table(Table<Cell>& search_table, int ny, int nx);
    ///Table<Cell> wrap_table(Table<Cell>& search_table, int ny, int nx, int& score);
    bool is_empty_table(const Table<Cell>& search_table, int ny, int nx);
    bool is_walkable_table(const Table<Cell>& search_table, int ny, int nx);

private:
    void rotate_clockwise();
    void rotate_counterclockwise();

    void wrap();

    void dfs(int cy, int cx);

    bool is_empty(int cy, int cx);

    bool is_inside(int cy, int cx);

    int get_empty_neighrbor_cell(int cy, int cx);

    int get_empty_connected_cell(int cy, int cx, int upperbound);

    /// nf
    int get_empty_table_neighrbor_cell(const Table<Cell>& search_table, int cy, int cx);


    int idx;

    Table<Cell> table;
    int y;
    int x;

    int height() const;
    int width() const;

    bool reset_to_small_region();

    std::vector<Point> manipulator_list;

    std::vector<Action> action_list;

    std::array<int, 4> dx;
    std::array<int, 4> dy;
};

int Worker::select_shortest(std::vector<Point>& target_list)
{
    const Point current(y, x);
    int index = -1;
    int best_score = std::numeric_limits<int>::max();
    for (int i = 0; i < target_list.size(); i++)
    {
        const int score = shortest_distance_to(current, [&](const Point& p) -> bool { return target_list[i] == p; });
        if (best_score > score)
        {
            best_score = score;
            index = i;
        }
    }
    return index;
}

void Worker::collect_optimal(std::vector<Point>& target_list)
{
    const std::size_t dimension = target_list.size();

    std::vector<std::vector<int>> distance(dimension, std::vector<int>(dimension, 0));
    for (int i = 0; i < dimension; i++)
    {
        for (int j = i + 1; j < dimension; j++)
        {
            const int d = shortest_distance_to(target_list[i], [&](Point& p) { return p == target_list[j]; });
            distance[i][j] = distance[j][i] = d;
        }
    }

    auto tour = HCPSolver::solve(distance);

    for (auto& idx : tour)
    {
        const Point target = target_list[idx];

        bfs_move([&](Point& p) { return p == target; });

        if (table[target.y][target.x] == Cell::ManipulatorExtension)
        {
            attach_command();
        }
    }
}

void Worker::init(Table<Cell>& table, int y, int x)
{
    this->y = y;
    this->x = x;

    this->table.resize(table.size());
    for (int i = 0; i < table.size(); i++)
    {
        this->table[i].resize(table[i].size());
        for (int j = 0; j < table[0].size(); j++)
        {
            this->table[i][j] = table[i][j];
        }
    }

    // Up, Left, Down, Right
    dy = { 1, 0, -1, 0 };
    dx = { 0, -1, 0, 1 };

    manipulator_list.emplace_back(0, 0);
    manipulator_list.emplace_back(1, 1);
    manipulator_list.emplace_back(-1, 1);
    manipulator_list.emplace_back(0, 1);

    // wrap();
}

std::vector<Action>& Worker::get_action_list()
{
    return action_list;
}

Table<Cell>& Worker::get_table()
{
    return table;
}

std::vector<Point> Worker::gather(Cell cell)
{
    return gather_with<>([&](int y, int x) { return table[y][x] == cell; });
}

template <typename FUNCTION>
std::vector<Point> Worker::gather_with(FUNCTION condition)
{
    std::vector<Point> ret;

    for (int i = 0; i < height(); i++)
    {
        for (int j = 0; j < width(); j++)
        {
            if (condition(i, j))
            {
                ret.emplace_back(i, j);
            }
        }
    }
    return ret;
}

void Worker::copy_from(const Worker& src)
{
    // table
    table.resize(src.table.size());
    for (auto& row : table)
    {
        row.resize(src.width());
    }
    for (int i = 0; i < height(); i++)
    {
        for (int j = 0; j < width(); j++)
        {
            table[i][j] = src.table[i][j];
        }
    }

    y = src.y;
    x = src.x;

    for (int i = 0; i < 4; i++)
    {
        manipulator_list.push_back(src.manipulator_list[i]);
    }

    // action list doesn't have to share

    for (int i = 0; i < 4; i++)
    {
        dy[i] = src.dy[i];
        dx[i] = src.dx[i];
    }
}

Worker::Worker(const Worker& src)
{
    if (this != &src)
    {
        copy_from(src);
    }
}

Worker::Worker()
{
}

bool Worker::is_empty(int cy, int cx)
{
    return table[cy][cx] != Cell::Obstacle && table[cy][cx] != Cell::Occupied;
}

void Worker::attach_command()
{
    auto p = manipulator_list.back();
    Point zero(0, 0);
    auto dir = boardloader::get_direction(zero, p);
    p += dir;
    manipulator_list.push_back(p);
    action_list.emplace_back(ActionType::Attatch, p);
}

void Worker::clone_command()
{
    assert(table[y][x] == Cell::Mysterious);
    action_list.push_back(ActionType::Cloning);
}

void Worker::move(Direction dir)
{
    int index = static_cast<int>(dir);
    y += dy[index];
    x += dx[index];
    action_list.emplace_back(ActionType::Move, dir);
    //wrap();
}

bool Worker::is_inside(int cy, int cx)
{
    return 0 <= cy && cy < height() && 0 <= cx && cx < width();
}

int Worker::height() const
{
    return table.size();
}

int Worker::width() const
{
    return table[0].size();
}

void Worker::wrap()
{
    // TODO: 当たり判定を入れる
    for (int i = 0; i < manipulator_list.size(); i++)
    {
        auto& v = manipulator_list[i];
        const int ny = y + v.y;
        const int nx = x + v.x;

        if (is_inside(ny, nx))
        {
            if (i >= 3 && table[ny][nx] == Cell::Obstacle)
            {
                break;
            }
            else if (table[ny][nx] != Cell::Obstacle)
            {
                table[ny][nx] = Cell::Occupied;
            }
        }
    }
}

void Worker::rotate_clockwise()
{
    for (auto& p : manipulator_list)
    {
        const int py = p.y;
        const int px = p.x;
        p.y = -px;
        p.x = py;
    }
}

void Worker::rotate_counterclockwise()
{
    for (auto& p : manipulator_list)
    {
        const int py = p.y;
        const int px = p.x;
        p.y = px;
        p.x = -py;
    }
}

Worker::Worker(Table<Cell>& table, int y, int x)
{
    init(table, y, x);
}

Worker::Worker(Table<Cell>& table, int y, int x, int idx) : idx(idx)
{
    init(table, y, x);
}

template <typename FUNCTION>
int Worker::shortest_distance_to(const Point& start, FUNCTION condition)
{
    std::queue<std::pair<Point, int>> que;
    que.emplace(start, 0);

    Table<bool> visited(height(), std::vector<bool>(width(), false));
    visited[start.y][start.x] = true;

    while (!que.empty())
    {
        int depth;
        Point p;
        std::tie(p, depth) = que.front();
        que.pop();

        if (condition(p))
        {
            return depth;
        }

        for (int i = 0; i < 4; i++)
        {
            const int ny = p.y + dy[i];
            const int nx = p.x + dx[i];

            if (is_inside(ny, nx) && table[ny][nx] != Cell::Obstacle && !visited[ny][nx])
            {
                visited[ny][nx] = true;
                que.emplace(Point(ny, nx), depth + 1);
            }
        }
    }
    return 100000;
}

template <typename FUNCTION>
bool Worker::bfs_move(FUNCTION condition)
{
    Table<Direction> recur(height(), std::vector<Direction>(width(), Direction::None));
    // 何でも良い
    recur[y][x] = Direction::Left;

    std::queue<Point> que;
    que.emplace(y, x);

    while (!que.empty())
    {
        Point p = que.front();
        que.pop();

        if (condition(p))
        {
            std::vector<Direction> move_list;
            while (p.y != y || p.x != x)
            {
                const Direction inv_dir = recur[p.y][p.x];
                const int index = static_cast<int>(inv_dir);
                p.y += dy[index];
                p.x += dx[index];
                move_list.push_back(inverse(inv_dir));
            }
            std::reverse(move_list.begin(), move_list.end());
            for (auto m : move_list)
            {
                move(m);
            }
            return true;
        }

        for (int i = 0; i < 4; i++)
        {
            const int ny = p.y + dy[i];
            const int nx = p.x + dx[i];

            if (is_inside(ny, nx) && table[ny][nx] != Cell::Obstacle && recur[ny][nx] == Direction::None)
            {
                que.emplace(ny, nx);
                recur[ny][nx] = inverse(static_cast<Direction>(i));
            }
        }
    }
    return false;
}

int Worker::get_empty_connected_cell(int cy, int cx, int upperbound)
{
    std::queue<Point> que;
    que.emplace(cy, cx);

    Table<bool> visited(height(), std::vector<bool>(width(), false));

    int count = 0;

    while (!que.empty() && count < upperbound)
    {
        Point p = que.front();
        que.pop();

        for (int i = 0; i < 4; i++)
        {
            const int ny = p.y + dy[i];
            const int nx = p.x + dx[i];

            if (is_inside(ny, nx) && is_empty(ny, nx) && !visited[ny][nx])
            {
                visited[ny][nx] = true;
                que.emplace(ny, nx);
                count++;
            }
        }
    }
    return count;
}

int Worker::get_empty_neighrbor_cell(int cy, int cx)
{
    int count = 0;
    for (int i = 0; i < 4; i++)
    {
        const int ny = cy + dy[i];
        const int nx = cx + dx[i];
        if (is_inside(ny, nx) && is_empty(ny, nx))
        {
            count++;
        }
    }
    return count;
}

bool Worker::reset_to_small_region()
{
    for (auto rel_point : manipulator_list)
    {
        const int my = y + rel_point.y;
        const int mx = x + rel_point.x;

        for (int i = 0; i < 4; i++)
        {
            const int ny = my + dy[i];
            const int nx = mx + dx[i];

            constexpr int upperbound = 50;

            if (is_inside(ny, nx) && is_empty(ny, nx) && get_empty_connected_cell(ny, nx, upperbound) < upperbound)
            {
                // move (y, x) -> (ny, nx)
                bfs_move([&](Point& p) { return p.y == ny && p.x == nx; });
                return true;
            }
        }
    }
    return false;
}

Direction Worker::get_move_direction(char a)
{
    switch (a) {
        case 'W':
            return Direction::Up;
        case 'S':
            return Direction::Down;
        case 'A':
            return Direction::Left;
        case 'D':
            return Direction::Right;
        default:
            break;
    }
    return Direction::None;
}

Point Worker::nearest_cc(int y, int x)
{
    int max_check_count = 10;

    std::random_device seed_gen;
    std::mt19937 engine(seed_gen());

    std::vector<Point> area;
    int H = table.size();
    int W = table[0].size();
    unsigned int next_dist = -1;
    Point next_pos({-1, -1});
    for (const auto& space : ccl::spaces) {
        area.clear();
        for (int i = 0; i < ccl::data.size(); i++) {
            if (ccl::data[i] == space) {
                area.push_back(Point(i / W, i % W));
            }
        }
        std::shuffle(area.begin(), area.end(), engine);
        int cnt = 0;
        unsigned int longest_dist = 0;
        unsigned int nearest_dist = -1;
        Point longest_pos;
        Point nearest_pos;
        for (const Point& pos : area) {
        //for (const Point& pos : candidates) {
            unsigned int dist = ikeda::calc_distance(table, {y, x}, {pos.y, pos.x});
            if (longest_dist < dist) {
                longest_dist = dist;
                longest_pos = pos;
            }
            if (nearest_dist > dist) {
                nearest_dist = dist;
                nearest_pos = pos;
            }
            if (cnt++ > max_check_count) break;
        }
        if (next_dist > longest_dist) {
        //if (next_dist > nearest_dist) {
            next_dist = longest_dist;
            //next_pos = longest_pos;
            //next_dist = nearest_dist;
            next_pos = nearest_pos;
        }
    }

    return next_pos;
}

int Worker::get_empty_table_neighrbor_cell(const Table<Cell>& search_table, int cy, int cx)
{
    int count = 0;
    for (int i = 0; i < 4; i++)
    {
        const int ny = cy + dy[i];
        const int nx = cx + dx[i];
        if (is_inside(ny, nx) && is_empty_table(search_table, ny, nx))
        {
            count++;
        }
    }
    return count;
}

int Worker::wrap_table(Table<Cell>& search_table, int y_, int x_)
//Table<Cell> Worker::wrap_table(Table<Cell>& search_table, int y_, int x_, int& score)
{
    int score = 0;

    //search_table[y_][x_] = Cell::Occupied;
    
    for (int i = 0; i < manipulator_list.size(); i++)
    //for (int i = 1; i < manipulator_list.size(); i++)
    {
        auto& v = manipulator_list[i];
        const int ny = y_ + v.y;
        const int nx = x_ + v.x;

        if (is_inside(ny, nx))
        {
            ///if (search_table[ny][nx] == Cell::Occupied) score++; /// penalty

            if (i >= 3 && search_table[ny][nx] == Cell::Obstacle)
            {
                break;
            }
            else if (search_table[ny][nx] != Cell::Obstacle)
            {
                search_table[ny][nx] = Cell::Occupied;
                //score--;
            } else {
                score++; /// penalty
            }
        } else {
            score++; /// penalty
        }
    }

    return score;
    //return search_table;
}

bool Worker::is_empty_table(const Table<Cell>& search_table, int cy, int cx)
{
    return search_table[cy][cx] != Cell::Obstacle && search_table[cy][cx] != Cell::Occupied;
    //return search_table[cy][cx] != Cell::Obstacle;
}

bool Worker::is_walkable_table(const Table<Cell>& search_table, int cy, int cx)
{
    //return search_table[cy][cx] != Cell::Obstacle && search_table[cy][cx] != Cell::Occupied;
    return search_table[cy][cx] != Cell::Obstacle;
}

void print_table(Table<Cell>& t)
{
    for (const auto& xs : t) {
        for (const auto& x : xs) {
            std::cerr << static_cast<char>(x);
        }
        std::cerr << std::endl;
    }
    std::cerr << std::endl;
}

std::vector<pair<int, vector<Direction>>> search_paths;

void Worker::search(Table<Cell>& search_table, int y, int x, int score, std::vector<Direction>& path)
{
    const int max_search_depth = 10;

    if (path.size() > max_search_depth) {
        int last_score = 0;
        for (int i = 0; i < 4; i++) {
            const int ny = y + dy[i];
            const int nx = x + dx[i];
            if (is_inside(ny, nx) && is_empty_table(search_table, ny, nx)) {
                last_score++;
                //last_score--;
            }
        }
        search_paths.push_back(make_pair(score + last_score, path));
        //search_paths.push_back(make_pair(score, path));
        return;
    }

    //std::cerr << "in-search!" << std::endl; ///// debug
    bool is_inside_check = false;
    Table<Cell> prev_search_table;
    for (int i = 0; i < 4; i++) {
        const int ny = y + dy[i];
        const int nx = x + dx[i];
        ///int wrap_score = 0;
        //int wrap_score = wrap_table(search_table, ny, nx);
        ///search_table = wrap_table(search_table, ny, nx, wrap_score);
        //std::cerr << "depth: " << path.size() << " (" << ny << "," << nx << ")" << std::endl; ///// debug
        //print_table(search_table); ///// debug
        if (is_inside(ny, nx) && is_empty_table(search_table, ny, nx)) {
        //if (is_inside(ny, nx) && is_walkable_table(search_table, ny, nx)) {
            //std::cerr << "in-search" << std::endl; ///// debug
            is_inside_check = true;
            prev_search_table = search_table;
            int wrap_score = wrap_table(search_table, ny, nx);
            const Direction dir = static_cast<Direction>(i);
            ///wrap();
            path.push_back(dir);
            //int next_score = score + wrap_score;
            int next_score = score + wrap_score + get_empty_table_neighrbor_cell(search_table, ny, nx);
            //search(search_table, ny, nx, score + wrap_score + get_empty_table_neighrbor_cell(search_table, ny, nx), path);
            search(search_table, ny, nx, next_score, path);
            path.pop_back();
            search_table = prev_search_table;
        }
    }
    if (!is_inside_check && !path.empty()) {
        search_paths.push_back(make_pair(score, path));
    }
    /*
    Direction dir;
    int index = static_cast<int>(dir);
    y += dy[index];
    x += dx[index];
    action_list.emplace_back(ActionType::Move, dir);
    */
}

static int count_search = 0;
std::vector<Direction> best_path;

void Worker::search_depth(Table<Cell>& search_table, int y, int x, int score, std::vector<Direction>& path)
{
    if (path.size() > best_path.size()) {
        best_path = path;
    }

    if (count_search++ > 50) {
        return;
    }

    bool is_inside_check = false;
    Table<Cell> prev_search_table;
    for (int i = 0; i < 4; i++) {
        const int ny = y + dy[i];
        const int nx = x + dx[i];
        prev_search_table = search_table;
        int wrap_score = wrap_table(search_table, ny, nx);
        //int wrap_score = 0;
        //search_table = wrap_table(search_table, ny, nx, wrap_score);
        if (is_inside(ny, nx) && is_empty_table(search_table, ny, nx)) {
            is_inside_check = true;
            const Direction dir = static_cast<Direction>(i);
            path.push_back(dir);
            //int next_score = score + wrap_score + get_empty_table_neighrbor_cell(search_table, ny, nx);

            search_depth(search_table, ny, nx, score, path);
            path.pop_back();
        }
        search_table = prev_search_table;
    }
}

void Worker::dfs_with_restart()
{
    std::vector<std::pair<int, Direction>> selected;

    while (true)
    {
        wrap();

        //if (!reset_to_small_region())
        if (true)
        {
            selected.clear();

            ///wrap(); ///
            //table[y][x] == Cell::Occupied; ///// debug
            Table<Cell> search_table = table;
            std::vector<Direction> search_path;
            search_paths.clear();
            //count_search = 0;
            //best_path.clear();
            search(search_table, y, x, 0, search_path);
            //search_depth(search_table, y, x, 0, search_path);
            //search_path = best_path;

#if 0
            for (int i = 0; i < 4; i++)
            {
                const int ny = y + dy[i];
                const int nx = x + dx[i];
                if (is_inside(ny, nx) && is_empty(ny, nx))
                {
                    const Direction dir = static_cast<Direction>(i);
                    selected.emplace_back(get_empty_neighrbor_cell(ny, nx), dir);
                }
            }
#endif
            ///if (selected.empty())
            if (search_paths.empty())
            {
#if 0
                ccl::check_ccl(table, ccl::data, ccl::spaces); ///// debug
                std::cerr << "number of connected components [" << idx << "]: " << ccl::spaces.size() << std::endl;

                Point next_pos(nearest_cc(y, x));
                //Point next_pos(largest_cc(y, x));
                ///std::cerr << "next_pos: " << next_pos.x << ", " << next_pos.y << std::endl;
                if (next_pos.x == -1) break;
                std::string move_string = ikeda::move(table, {y, x}, {next_pos.y, next_pos.x});
                for (char a : move_string) {
                    move(get_move_direction(a));
                    wrap();
                }
#else
                if (!bfs_move([&](Point& p) { return is_empty(p.y, p.x); }))
                {
                    break;
                }
#endif
            }
            else
            {
#if 0
                sort(selected.begin(), selected.end());
                move(selected[0].second);
#endif
                sort(search_paths.begin(), search_paths.end());
                pair<int, vector<Direction>> target(search_paths.front());
            
                /// debug
                std::cerr << "search_paths: " << search_paths.size();
                if (!search_paths.empty()) {
                    std::cerr << " : " << target.first << " : "; /////
                    for (const auto& d : target.second) {
                        std::cerr << direction_to_string(d);
                    }
                }
                std::cerr << std::endl;

                ///target.second.pop_back(); /// debug
                for (const auto& d : target.second) {
                    move(d);
                    wrap();
                }
            }
        }
    }
}

void Worker::dfs(int cy, int cx)
{
    table[cy][cx] = Cell::Occupied;

    for (int i = 0; i < 4; i++)
    {
        const int ny = cy + dy[i];
        const int nx = cx + dx[i];
        if (is_inside(ny, nx) && is_empty(ny, nx))
        {
            const Direction dir = static_cast<Direction>(i);
            move(dir);
            dfs(ny, nx);
            move(inverse(dir));
        }
    }
}

std::vector<Action> Worker::solve()
{
    dfs_with_restart();
    // dfs(y, x);

    return action_list;
}

class Solver
{
public:
    Solver(Table<Cell> table, int start_y, int start_x, const std::vector<char>& items);

    std::vector<std::vector<Action>> solve();

    void fill_obstacle(const Table<int>& cluster, Table<Cell>& table, int id);

    std::vector<Point> generate_base_point_list(Worker& worker, int number_cluster);

private:
    std::vector<Worker> worker_list;
    Cluster cluster;
    
    int count_clones;
    int count_manips;
};

std::vector<Point> Solver::generate_base_point_list(Worker& worker, int cluster_count)
{
    auto& table = worker.get_table();

    std::vector<Point> candidate;
    for (int i = 0; i < table.size(); i++)
    {
        for (int j = 0; j < table[0].size(); j++)
        {
            if (table[i][j] != Cell::Obstacle)
            {
                candidate.emplace_back(i, j);
            }
        }
    }
    std::mt19937_64 mt;

    std::shuffle(candidate.begin(), candidate.end(), mt);
    std::uniform_int_distribution<int> rand(0, candidate.size() - 1);

    std::vector<Point> best_input(cluster_count);
    int best_eval = std::numeric_limits<int>::max();

    std::vector<Point> input(cluster_count);

    for (int i = 0; i < 3000; i++)
    {
        for (int j = 0; j < cluster_count; j++)
        {
            input[j] = candidate[rand(mt)];
        }
        auto clusters = cluster.cluster_count_by_bfs(table, input);
        const int eval = *std::max_element(clusters.begin(), clusters.end()) - *std::min_element(clusters.begin(), clusters.end());
        if (best_eval > eval)
        {
            best_eval = eval;
            std::copy(input.begin(), input.end(), best_input.begin());
        }
    }
    return best_input;
}

void Solver::fill_obstacle(const Table<int>& cluster, Table<Cell>& table, int id)
{
    const int height = cluster.size();
    const int width = cluster[0].size();

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (cluster[i][j] != id)
            {
                table[i][j] = Cell::Obstacle;
            }
        }
    }
}

Solver::Solver(Table<Cell> table, int start_y, int start_x, const std::vector<char>& items)
    : count_clones(0), count_manips(0)
{
    worker_list.resize(1000);
    int idx = 0;
    for (auto& worker : worker_list) {
        worker.set_index(idx++);
    }
    worker_list[0].init(table, start_y, start_x);

    for (const auto item : items) {
        if (item == 'C') count_clones++;
        if (item == 'B') count_manips++;
    }
    std::cerr << "clones (buy): " << count_clones << std::endl;
    std::cerr << "manips (buy): " << count_manips << std::endl;
}

std::vector<std::vector<Action>> Solver::solve()
{
    // manipulator を集めて、前にくっつける + Clone 回収

    std::vector<Point> clone_point = worker_list[0].gather(Cell::Cloning);
    std::vector<Point> manipulator_point = worker_list[0].gather(Cell::ManipulatorExtension);
    std::vector<Point> mysterious_point = worker_list[0].gather(Cell::Mysterious);

    std::vector<Point> target_list;
    for (auto& p : clone_point)
    {
        target_list.push_back(p);
    }
    for (auto& p : manipulator_point)
    {
        target_list.push_back(p);
    }

    int worker_size = 1;

    if (target_list.size() == 0 && count_clones == 0 && count_manips == 0)
    {
        worker_list[0].dfs_with_restart();
    }
    else
    {
        for (int i = 0; i < count_manips; i++) {
            worker_list[0].attach_command();
        }

        if (target_list.size() > 0) { /// nf
        int nearest_index = worker_list[0].select_shortest(target_list);
        if (1 < target_list.size())
        {
            int opponent = nearest_index == 0 ? 1 : 0;
            std::swap(target_list[nearest_index], target_list[opponent]);
        }
        worker_list[0].collect_optimal(target_list);
        }

        //const int num_clone = clone_point.size();
        const int num_clone = clone_point.size() + count_clones; /// nf

        // Clone
        worker_size = num_clone + 1;

        // Clustering の結果を収集
        if (num_clone > 0)
        {
            std::vector<Point> base_point_list = generate_base_point_list(worker_list[0], num_clone + 1);

            Table<int> clustering_result = cluster.clustering_by_bfs(worker_list[0].get_table(), base_point_list);

            const int nearest_mysterious = worker_list[0].select_shortest(mysterious_point);
            worker_list[0].bfs_move([&](Point& p) { return p == mysterious_point[nearest_mysterious]; });

            for (int i = 1; i <= num_clone; i++)
            {
                worker_list[i].copy_from(worker_list[0]);
                worker_list[0].clone_command();

                worker_list[i].bfs_move([&](Point& p) { return p == base_point_list[i]; });
                fill_obstacle(clustering_result, worker_list[i].get_table(), i);
                worker_list[i].dfs_with_restart();
            }

            worker_list[0].bfs_move([&](Point& p) { return p == base_point_list[0]; });
            fill_obstacle(clustering_result, worker_list[0].get_table(), 0);
        }
        worker_list[0].dfs_with_restart();
    }

    std::vector<std::vector<Action>> ret;
    for (int i = 0; i < worker_size; i++)
    {
        ret.push_back(worker_list[i].get_action_list());
    }

    return ret;
}

} // namespace xyzworker

void print_action(const std::vector<std::vector<xyzworker::Action>>& action_table, std::ostream& out)
{
    int action_size = 0; ///
    bool first = true;
    for (auto& action_list : action_table)
    {
        if (!first)
        {
            out << "#";
            action_size += 1;
        }
        first = false;
        action_size += action_list.size();
        for (auto action : action_list)
        {
            out << action.to_string();
        }
    }
    out << std::endl;

    std::cerr << "Size: " << action_size << std::endl; /// debug
}

void save_action(const std::vector<std::vector<xyzworker::Action>>& action_table, const std::string& filepath)
{
    std::ofstream fout;
    fout.open(filepath);

    if (!fout.is_open())
    {
        std::cerr << "cannot open file @ save_action." << std::endl;
        return;
    }

    print_action(action_table, fout);

    fout.close();
}

int main(int argc, char* argv[])
{
    std::string input_filepath(argv[1]);
    std::string option_filepath("");
    if (argc > 2) option_filepath = argv[2];

    int start_y, start_x;
    auto board = load_board(input_filepath, start_y, start_x);

    std::vector<char> items(0);
    if (!option_filepath.empty()) {
        std::stringstream ss;
        std::string line;
        std::fstream fs(option_filepath, std::ios_base::in);
        std::getline(fs, line);
        ss.str(line);
        char c;
        for (ss.str(line); ss >> c; items.push_back(c));
    }

    xyzworker::Solver solver(board, start_y, start_x, items);

    auto result = solver.solve();

    print_action(result, std::cout);
}

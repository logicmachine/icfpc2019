#include "board_loader.hpp"

#include <algorithm>
#include <memory>
#include <queue>
#include <random>
#include <stack>
#include <tuple>
#include <unordered_set>

using boardloader::Cell;
using boardloader::ItemCounter;
using boardloader::load_board;
using boardloader::Point;
using boardloader::print_table;
using boardloader::Table;

namespace xyzworker
{

std::array<std::array<std::uint64_t, 401>, 401> place_hash_table;
std::array<std::array<std::uint64_t, 401>, 401> player_hash_table;

void initialize_hash(std::array<std::array<std::uint64_t, 401>, 401>& hash_table, std::mt19937_64& mt)
{
    for (auto& row : hash_table)
    {
        for (auto& item : row)
        {
            item = mt();
        }
    }
}

class ManhattanNeighbor
{
public:
    static constexpr std::size_t size = 4;
    static std::array<int, 4> dx;
    static std::array<int, 4> dy;
};
// Up, Left, Down, Right
std::array<int, 4> ManhattanNeighbor::dy = { 1, 0, -1, 0 };
std::array<int, 4> ManhattanNeighbor::dx = { 0, -1, 0, 1 };

class ChebyshevNeighbor
{
public:
    static constexpr std::size_t size = 8;
    static std::array<int, 8> dx;
    static std::array<int, 8> dy;
};

std::array<int, 8> ChebyshevNeighbor::dy = { 1, 0, -1, 0, -1, 1, -1, 1 };
std::array<int, 8> ChebyshevNeighbor::dx = { 0, 1, 0, -1, 1, 1, -1, -1 };

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
    using Metric = ManhattanNeighbor;

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

        for (int i = 0; i < Metric::size; i++)
        {
            const int ny = p.y + Metric::dy[i];
            const int nx = p.x + Metric::dx[i];

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

bool is_orthogonal(const Direction dir1, const Direction dir2)
{
    return (static_cast<int>(dir1) % 2) != (static_cast<int>(dir2) % 2);
}

Point to_point(const Direction direction)
{
    switch (direction)
    {
    case Direction::Up:
        return Point(1, 0);
    case Direction::Left:
        return Point(0, -1);
    case Direction::Down:
        return Point(-1, 0);
    case Direction::Right:
        return Point(0, 1);
    }
    assert(false);
}

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
    NearestMove,
};

struct Action
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

    std::string to_string() const
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

void print_action(const std::vector<std::vector<xyzworker::Action>>& action_table, std::ostream& out);

class Worker;

void beam_search(Worker* init, int max_turn, int beam_width, const std::vector<Point>& point_to_optimize,
    std::vector<Action>& action_list);

class Worker
{
public:
    std::vector<Action> solve();

    void init(Table<Cell>& table, int y, int x, const ItemCounter& init_itemc_counter);

    ~Worker();

    Worker();
    Worker(const Worker& src);
    void copy_from(const Worker& src);
    void copy_from_complete(const Worker& src);

    void apply(Action& action);
    void undo(Action& action);

    Table<Cell>& get_table();
    const Table<Cell>& get_table() const;

    std::vector<Action>& get_action_list();

    std::vector<Point> gather(Cell cell);

    template <typename FUNCTION>
    std::vector<Point> gather_with(FUNCTION condition);

    template <typename FUNCTION>
    Point bfs_move_point(FUNCTION condition);

    template <typename FUNCTION>
    std::vector<Direction> bfs_move(FUNCTION condition);

    template <typename FUNCTION>
    bool bfs_move_with_move(FUNCTION condition);

    template <typename FUNCTION>
    int shortest_distance_to(const Point& start, FUNCTION condition);

    void dfs_with_restart();

    void divide_and_search();

    std::vector<Point> select_optimize_region();

    void collect_optimal(std::vector<Point>& target_list);

    int select_shortest(std::vector<Point>& target_list);

    // Command

    bool is_valid_move(const Action& action);

    void move(Direction dir);

    void unmove(Direction dir);

    void clone_command();

    void attach_command();

    void rotate_clockwise();

    void unrotate_clockwise();

    void rotate_counterclockwise();

    void unrotate_counterclockwise();

    ItemCounter item_counter;

    void setup_hash();

    std::uint64_t hash() const;

    void print_table() const;

    bool is_occupied(int cy, int cx) const;

private:
    void wrap();
    void unwrap();

    void dfs(int cy, int cx);

    bool is_empty(int cy, int cx);

    bool is_inside(int cy, int cx);

    int should_rotate_clockwise(Direction dir, int length);

    int count_pass_empty(const Point& base, Direction move_dir, int move_length, Direction side_dir, int side_length);

    int get_empty_neighrbor_cell(int cy, int cx);

    int get_empty_connected_cell_count(int cy, int cx, int upperbound);

    std::vector<Point> get_empty_connected_cell(int cy, int cx, int upperbound);

    void generate_move_sequence(const std::vector<Direction>& move_list);

    Table<int> occupied;
    Table<Cell> table;
    int y;
    int x;

    Direction player_direction;

    int height() const;
    int width() const;

    bool reset_to_small_region();

    std::vector<Point> manipulator_list;

    std::vector<Action> action_list;

    std::uint64_t _hash;
};

void Worker::print_table() const
{
    Table<Cell> table_for_print(height(), std::vector<Cell>(width()));
    for (int i = 0; i < height(); i++)
    {
        for (int j = 0; j < width(); j++)
        {
            table_for_print[i][j] = table[i][j];
            if (occupied[i][j] > 0)
            {
                table_for_print[i][j] = Cell::Occupied;
            }
        }
    }
    boardloader::print_table(table_for_print, y, x);
}

std::uint64_t Worker::hash() const
{
    return _hash;
}

bool Worker::is_valid_move(const Action& action)
{
    switch (action.type)
    {
    case ActionType::Move:
    {
        const auto dp = to_point(action.direction);
        const int ny = y + dp.y;
        const int nx = x + dp.x;
        return is_inside(ny, nx) && table[ny][nx] != Cell::Obstacle;
    }

    case ActionType::TurnClockwise:
    case ActionType::TurnCounterClockwise:
        return true;
    }
    assert(false);
}

bool Worker::is_occupied(int cy, int cx) const
{
    return occupied[cy][cx] > 0;
}

void Worker::apply(Action& action)
{
    switch (action.type)
    {
    case ActionType::Move:
        move(action.direction);
        break;
    case ActionType::TurnClockwise:
        rotate_clockwise();
        break;
    case ActionType::TurnCounterClockwise:
        rotate_counterclockwise();
        break;
    }
}

void Worker::undo(Action& action)
{
    switch (action.type)
    {
    case ActionType::Move:
        unmove(action.direction);
        break;
    case ActionType::TurnClockwise:
        unrotate_clockwise();
        break;
    case ActionType::TurnCounterClockwise:
        unrotate_counterclockwise();
        break;
    }
}

void Worker::setup_hash()
{
    _hash = 0;
    for (int i = 0; i < height(); i++)
    {
        for (int j = 0; j < width(); j++)
        {
            if (is_empty(i, j))
            {
                _hash ^= place_hash_table[i][j];
            }
        }
    }
    _hash ^= player_hash_table[y][x];
    _hash += static_cast<std::uint64_t>(player_direction);
}

Worker::~Worker()
{
}

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

        bfs_move_with_move([&](Point& p) { return p == target; });

        if (table[target.y][target.x] == Cell::ManipulatorExtension)
        {
            attach_command();
        }
    }
}

void Worker::init(Table<Cell>& table, int y, int x, const ItemCounter& init_item_counter)
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

    occupied.resize(height());
    for (auto& row : occupied)
    {
        row.resize(width(), 0);
    }

    manipulator_list.emplace_back(0, 0);
    manipulator_list.emplace_back(1, 1);
    manipulator_list.emplace_back(-1, 1);
    manipulator_list.emplace_back(0, 1);

    wrap();

    item_counter.reset();
    item_counter += init_item_counter;

    player_direction = Direction::Right;
}

std::vector<Action>& Worker::get_action_list()
{
    return action_list;
}

Table<Cell>& Worker::get_table()
{
    return table;
}

const Table<Cell>& Worker::get_table() const
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

void Worker::copy_from_complete(const Worker& src)
{
    // table
    table.resize(src.table.size());
    for (auto& row : table)
    {
        row.resize(src.width());
    }

    occupied.resize(height());
    for (auto& row : occupied)
    {
        row.resize(width(), 0);
    }

    for (int i = 0; i < height(); i++)
    {
        for (int j = 0; j < width(); j++)
        {
            table[i][j] = src.table[i][j];
            occupied[i][j] = src.occupied[i][j];
        }
    }

    y = src.y;
    x = src.x;

    player_direction = src.player_direction;

    manipulator_list.clear();
    for (auto& p : src.manipulator_list)
    {
        manipulator_list.push_back(p);
    }

    item_counter.reset();
    item_counter += src.item_counter;

    action_list.clear();
    for (auto& action : src.action_list)
    {
        action_list.push_back(action);
    }

    _hash = src.hash();
}

void Worker::copy_from(const Worker& src)
{
    // table
    table.resize(src.table.size());
    for (auto& row : table)
    {
        row.resize(src.width());
    }

    occupied.resize(height());
    for (auto& row : occupied)
    {
        row.resize(width(), 0);
    }

    for (int i = 0; i < height(); i++)
    {
        for (int j = 0; j < width(); j++)
        {
            table[i][j] = src.table[i][j];
            occupied[i][j] = src.occupied[i][j];
        }
    }

    y = src.y;
    x = src.x;

    manipulator_list.emplace_back(0, 0);
    manipulator_list.emplace_back(1, 1);
    manipulator_list.emplace_back(-1, 1);
    manipulator_list.emplace_back(0, 1);

    // action list doesn't have to share

    item_counter.reset();

    player_direction = Direction::Right;

    setup_hash();
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
    return table[cy][cx] != Cell::Obstacle && occupied[cy][cx] == 0;
}

void Worker::attach_command()
{
    assert(item_counter.get(Cell::ManipulatorExtension) > 0);
    item_counter.decrement(Cell::ManipulatorExtension);

    auto p = manipulator_list.back();
    Point zero(0, 0);
    auto dir = boardloader::get_direction(zero, p);
    p += dir;
    manipulator_list.push_back(p);
    action_list.emplace_back(ActionType::Attatch, p);
}

void Worker::clone_command()
{
    assert(item_counter.get(Cell::Cloning) > 0);
    assert(table[y][x] == Cell::Mysterious);
    action_list.push_back(ActionType::Cloning);
    item_counter.decrement(Cell::Cloning);
}

void Worker::unmove(Direction dir)
{
    if (table[y][x] != Cell::Empty)
    {
        item_counter.decrement(table[y][x]);
    }

    unwrap();

    action_list.pop_back();

    _hash ^= player_hash_table[y][x];

    int index = static_cast<int>(dir);
    y -= ManhattanNeighbor::dy[index];
    x -= ManhattanNeighbor::dx[index];

    _hash ^= player_hash_table[y][x];
}

void Worker::move(Direction dir)
{
    _hash ^= player_hash_table[y][x];

    int index = static_cast<int>(dir);
    y += ManhattanNeighbor::dy[index];
    x += ManhattanNeighbor::dx[index];

    assert(is_inside(y, x));
    assert(table[y][x] != Cell::Obstacle);

    action_list.emplace_back(ActionType::Move, dir);

    _hash ^= player_hash_table[y][x];

    wrap();

    if (table[y][x] != Cell::Empty)
    {
        item_counter.increment(table[y][x]);
    }
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
                occupied[ny][nx]++;
                if (occupied[ny][nx] == 1)
                {
                    _hash ^= place_hash_table[ny][nx];
                }
            }
        }
    }
}

void Worker::unwrap()
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
                occupied[ny][nx]--;
                if (occupied[ny][nx] == 0)
                {
                    _hash ^= place_hash_table[ny][nx];
                }
            }
        }
    }
}

void Worker::unrotate_clockwise()
{
    action_list.pop_back();

    unwrap();

    for (auto& p : manipulator_list)
    {
        const int py = p.y;
        const int px = p.x;
        p.y = px;
        p.x = -py;
    }
    _hash -= static_cast<std::uint64_t>(player_direction);
    player_direction = counter_clockwise(player_direction);
    _hash += static_cast<std::uint64_t>(player_direction);
}

void Worker::unrotate_counterclockwise()
{
    action_list.pop_back();

    unwrap();

    for (auto& p : manipulator_list)
    {
        const int py = p.y;
        const int px = p.x;
        p.y = -px;
        p.x = py;
    }

    _hash -= static_cast<std::uint64_t>(player_direction);
    player_direction = clockwise(player_direction);
    _hash += static_cast<std::uint64_t>(player_direction);
}

void Worker::rotate_clockwise()
{
    _hash -= static_cast<std::uint64_t>(player_direction);
    player_direction = clockwise(player_direction);
    _hash += static_cast<std::uint64_t>(player_direction);

    for (auto& p : manipulator_list)
    {
        const int py = p.y;
        const int px = p.x;
        p.y = -px;
        p.x = py;
    }

    wrap();

    action_list.emplace_back(ActionType::TurnClockwise);
}

void Worker::rotate_counterclockwise()
{
    _hash -= static_cast<std::uint64_t>(player_direction);
    player_direction = counter_clockwise(player_direction);
    _hash += static_cast<std::uint64_t>(player_direction);

    for (auto& p : manipulator_list)
    {
        const int py = p.y;
        const int px = p.x;
        p.y = px;
        p.x = -py;
    }

    wrap();

    action_list.emplace_back(ActionType::TurnCounterClockwise);
}

template <typename FUNCTION>
int Worker::shortest_distance_to(const Point& start, FUNCTION condition)
{
    using Metric = ManhattanNeighbor;

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

        for (int i = 0; i < Metric::size; i++)
        {
            const int ny = p.y + Metric::dy[i];
            const int nx = p.x + Metric::dx[i];

            if (is_inside(ny, nx) && table[ny][nx] != Cell::Obstacle && !visited[ny][nx])
            {
                visited[ny][nx] = true;
                que.emplace(Point(ny, nx), depth + 1);
            }
        }
    }
    return 100000;
}

int Worker::count_pass_empty(const Point& base, Direction move_dir, int move_length, Direction side_dir, int side_length)
{
    int count = 0;

    Point dm = to_point(move_dir);
    Point ds = to_point(side_dir);

    Point cur(base);

    for (int i = 0; i < move_length; i++)
    {
        Point cur2(cur);
        for (int j = 0; j < side_length; j++)
        {
            if (is_inside(cur2.y, cur2.x) && is_empty(cur2.y, cur2.x))
            {
                count++;
            }
            else if (table[y][x] == Cell::Obstacle)
            {
                break;
            }
            cur2 += ds;
        }
        cur += dm;
    }
    return count;
}

int Worker::should_rotate_clockwise(Direction move_dir, int move_length)
{
    Point current(y, x);
    const int side_length = manipulator_list.size() - 3;

    const int eval_clockwise = count_pass_empty(current, move_dir, move_length, clockwise(player_direction), side_length);
    const int eval_counterclockwise = count_pass_empty(current, move_dir, move_length, counter_clockwise(player_direction), side_length);

    if (eval_clockwise == 0 && eval_counterclockwise == 0)
    {
        return 0;
    }
    else if (eval_clockwise > eval_counterclockwise)
    {
        return 1;
    }
    else
    {
        return -1;
    }

    return eval_clockwise > eval_counterclockwise;
}

void Worker::generate_move_sequence(const std::vector<Direction>& move_list)
{
    int index = 0;
    while (index < move_list.size())
    {
        int begin = index;
        while (index < move_list.size() && move_list[begin] == move_list[index])
        {
            index++;
        }
        if (index - begin > 2)
        {
            if (manipulator_list.size() > 4)
            {
                if (!is_orthogonal(player_direction, move_list[begin]))
                {
                    int judge = should_rotate_clockwise(move_list[begin], index - begin);
                    if (judge > 0)
                    {
                        rotate_clockwise();
                    }
                    else if (judge < 0)
                    {
                        rotate_counterclockwise();
                    }
                }
            }
            else
            {
                if (is_orthogonal(player_direction, move_list[begin]))
                {
                    rotate_counterclockwise();
                }
            }
        }
        for (int i = begin; i < index; i++)
        {
            move(move_list[i]);
        }
    }
}

template <typename FUNCTION>
Point Worker::bfs_move_point(FUNCTION condition)
{
    auto move_list = bfs_move(condition);
    Point cur(y, x);
    for (auto& v : move_list)
    {
        cur += to_point(v);
    }
    return cur;
}

template <typename FUNCTION>
bool Worker::bfs_move_with_move(FUNCTION condition)
{
    auto move_list = bfs_move(condition);
    if (!move_list.empty())
    {
        generate_move_sequence(move_list);
        return true;
    }
    else
    {
        return false;
    }
}

template <typename FUNCTION>
std::vector<Direction> Worker::bfs_move(FUNCTION condition)
{
    using Metric = ManhattanNeighbor;

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
                p.y += Metric::dy[index];
                p.x += Metric::dx[index];
                move_list.push_back(inverse(inv_dir));
            }
            std::reverse(move_list.begin(), move_list.end());
            return move_list;
        }

        for (int i = 0; i < Metric::size; i++)
        {
            const int ny = p.y + Metric::dy[i];
            const int nx = p.x + Metric::dx[i];

            if (is_inside(ny, nx) && table[ny][nx] != Cell::Obstacle && recur[ny][nx] == Direction::None)
            {
                que.emplace(ny, nx);
                recur[ny][nx] = inverse(static_cast<Direction>(i));
            }
        }
    }
    std::vector<Direction> ret;
    return ret;
}

int Worker::get_empty_connected_cell_count(int cy, int cx, int upperbound)
{
    auto ret = get_empty_connected_cell(cy, cx, upperbound);
    return ret.size();
}

std::vector<Point> Worker::get_empty_connected_cell(int cy, int cx, int upperbound)
{
    using Metric = ManhattanNeighbor;

    std::queue<Point> que;
    que.emplace(cy, cx);

    Table<bool> visited(height(), std::vector<bool>(width(), false));

    std::vector<Point> ret;
    ret.emplace_back(cy, cx);

    while (!que.empty() && ret.size() < upperbound)
    {
        Point p = que.front();
        que.pop();

        for (int i = 0; i < Metric::size; i++)
        {
            const int ny = p.y + Metric::dy[i];
            const int nx = p.x + Metric::dx[i];

            if (is_inside(ny, nx) && is_empty(ny, nx) && !visited[ny][nx])
            {
                visited[ny][nx] = true;
                que.emplace(ny, nx);
                ret.emplace_back(ny, nx);
            }
        }
    }
    return ret;
}

int Worker::get_empty_neighrbor_cell(int cy, int cx)
{
    using Metric = ManhattanNeighbor;

    int count = 0;
    for (int i = 0; i < Metric::size; i++)
    {
        const int ny = cy + Metric::dy[i];
        const int nx = cx + Metric::dx[i];
        if (is_inside(ny, nx) && is_empty(ny, nx))
        {
            count++;
        }
    }
    return count;
}

bool Worker::reset_to_small_region()
{
    using Metric = ManhattanNeighbor;

    for (auto rel_point : manipulator_list)
    {
        const int my = y + rel_point.y;
        const int mx = x + rel_point.x;

        for (int i = 0; i < Metric::size; i++)
        {
            const int ny = my + Metric::dy[i];
            const int nx = mx + Metric::dx[i];

            constexpr int upperbound = 50;

            if (is_inside(ny, nx) && is_empty(ny, nx) && get_empty_connected_cell_count(ny, nx, upperbound) < upperbound)
            {
                // move (y, x) -> (ny, nx)
                bfs_move_with_move([&](Point& p) { return p.y == ny && p.x == nx; });
                return true;
            }
        }
    }
    return false;
}

std::vector<Point> Worker::select_optimize_region()
{
    const bool has_empty = bfs_move([&](Point& p) { return is_empty(p.y, p.x); }).size() > 0;
    if (has_empty)
    {
        auto empty_nearest = bfs_move_point([&](Point& p) { return is_empty(p.y, p.x); });

        auto target_point_list = get_empty_connected_cell(empty_nearest.y, empty_nearest.x, 20);

        std::vector<Cell> recur;
        for (auto& v : target_point_list)
        {
            recur.push_back(table[v.y][v.x]);
        }

        for (auto& v : target_point_list)
        {
            table[v.y][v.x] = Cell::Obstacle;
        }

        std::vector<Point> additional;

        // 細かい点をついでに集める
        for (auto& p : target_point_list)
        {
            using Metric = ManhattanNeighbor;
            for (int k = 0; k < Metric::size; k++)
            {
                const int ny = p.y + Metric::dy[k];
                const int nx = p.x + Metric::dx[k];
                if (is_inside(ny, nx) && is_empty(ny, nx))
                {
                    constexpr int upperbound = 20;
                    auto cand = get_empty_connected_cell(ny, nx, upperbound);
                    if (cand.size() < upperbound)
                    {
                        for (auto& v : cand)
                        {
                            additional.push_back(v);
                        }
                    }
                }
            }
        }
        for (auto& v : additional)
        {
            target_point_list.push_back(v);
        }

        for (int i = 0; i < recur.size(); i++)
        {
            const auto& p = target_point_list[i];
            table[p.y][p.x] = recur[i];
        }

        // if not empty
        if (!target_point_list.empty())
        {
            bfs_move_with_move([&](Point& p) { return p == empty_nearest; });
        }

        // max size of beam search
        return target_point_list;
    }
    else
    {
        std::vector<Point> ret;
        return ret;
    }
}

void Worker::divide_and_search()
{
    Table<bool> target_region(height(), std::vector<bool>(width(), false));

    while (true)
    {
        // select_region_to_optimize();
        std::vector<Point> region_to_optimize = select_optimize_region();

        if (region_to_optimize.empty())
        {
            break;
        }

        std::vector<Action> action_list;
        beam_search(this, 100, 5000, region_to_optimize, action_list);

        // apply
        for (auto& action : action_list)
        {
            apply(action);
        }
    }
}

void Worker::dfs_with_restart()
{
    using Metric = ManhattanNeighbor;

    std::vector<std::pair<int, Direction>> selected;

    while (true)
    {
        if (!reset_to_small_region())
        {
            selected.clear();

            for (int i = 0; i < Metric::size; i++)
            {
                const int ny = y + Metric::dy[i];
                const int nx = x + Metric::dx[i];
                if (is_inside(ny, nx) && is_empty(ny, nx))
                {
                    const Direction dir = static_cast<Direction>(i);
                    selected.emplace_back(get_empty_neighrbor_cell(ny, nx), dir);
                }
            }
            if (selected.empty())
            {
                if (!bfs_move_with_move([&](Point& p) { return is_empty(p.y, p.x); }))
                {
                    break;
                }
            }
            else
            {
                sort(selected.begin(), selected.end());
                move(selected[0].second);
            }
        }
    }
}

void Worker::dfs(int cy, int cx)
{
    using Metric = ManhattanNeighbor;

    occupied[cy][cx]++;

    for (int i = 0; i < Metric::size; i++)
    {
        const int ny = cy + Metric::dy[i];
        const int nx = cx + Metric::dx[i];
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

double evaluate(const Worker& worker, const std::vector<Point>& target_point)
{
    int count = 0;
    for (auto& p : target_point)
    {
        if (worker.is_occupied(p.y, p.x))
        {
            count++;
        }
    }
    return count;
}

struct WorkerDiff
{
    int worker_id;
    Action action;
    std::uint64_t rand;

    WorkerDiff(int worker_id, Action action, std::mt19937_64& mt)
        : worker_id(worker_id)
        , action(action)
        , rand(mt())
    {
    }
};

std::ostream& operator<<(std::ostream& out, const WorkerDiff& diff)
{
    out << "(" << diff.worker_id << ": " << diff.action.to_string() << ")";
    return out;
}

bool operator<(const WorkerDiff& d1, const WorkerDiff& d2)
{
    return d1.rand < d2.rand;
}

void beam_search(Worker* init, int max_turn, int beam_width, const std::vector<Point>& point_to_optimize,
    std::vector<Action>& action_list)
{
    std::mt19937_64 mt;
    int prev = 0;
    int next = 1;

    static std::vector<Worker> worker_list[2];

    static std::vector<std::pair<double, WorkerDiff>> state_list;

    worker_list[prev].resize(beam_width);
    worker_list[next].resize(beam_width);

    int active_worker_size = 1;
    state_list.clear();

    Worker temp;

    worker_list[prev][0].copy_from_complete(*init);
    worker_list[prev][0].get_action_list().clear();

    std::vector<Action> neighbor_move = {
        Action(ActionType::Move, Direction::Up),
        Action(ActionType::Move, Direction::Left),
        Action(ActionType::Move, Direction::Down),
        Action(ActionType::Move, Direction::Right),
        Action(ActionType::TurnClockwise),
        Action(ActionType::TurnCounterClockwise)
    };

    std::unordered_set<std::uint64_t> hash_set;

    double best_eval = 0;
    int min_turn = std::numeric_limits<int>::max();

    for (int turn = 0; turn < max_turn; turn++)
    {
        state_list.clear();
        for (int worker_id = 0; worker_id < active_worker_size; worker_id++)
        {
            temp.copy_from_complete(worker_list[prev][worker_id]);

            for (auto& action : neighbor_move)
            {
                if (temp.is_valid_move(action))
                {
                    auto prev_hash = temp.hash();
                    auto prev_action = temp.get_action_list().size();
                    assert(prev_action == turn);

                    temp.apply(action);
                    auto hash = temp.hash();
                    if (hash_set.find(hash) == hash_set.end())
                    {
                        // 大きいほどよい
                        double eval = evaluate(temp, point_to_optimize);

                        // 全部見つかった
                        if (eval == point_to_optimize.size())
                        {
                            action_list.clear();
                            for (auto& v : temp.get_action_list())
                            {
                                action_list.push_back(v);
                            }
                            return;
                        }

                        if (best_eval < eval || turn < min_turn)
                        {
                            best_eval = eval;
                            min_turn = turn;
                            action_list.clear();
                            for (auto& v : temp.get_action_list())
                            {
                                action_list.push_back(v);
                            }
                        }

                        state_list.emplace_back(-eval, WorkerDiff(worker_id, action, mt));
                        hash_set.insert(hash);
                    }
                    temp.undo(action);

                    auto rec_hash = temp.hash();
                    auto rec_action = temp.get_action_list().size();
                    assert(prev_action == rec_action);
                    assert(prev_hash == rec_hash);
                }
            }
            // nearest move action
        }
        std::sort(state_list.begin(), state_list.end());
        const int new_active_worker_size = std::min<int>(beam_width, state_list.size());
        for (int i = 0; i < new_active_worker_size; i++)
        {
            WorkerDiff diff = state_list[i].second;
            worker_list[next][i].copy_from_complete(worker_list[prev][diff.worker_id]);
            worker_list[next][i].apply(diff.action);
            assert(worker_list[next][i].get_action_list().size() == turn + 1);
        }

        active_worker_size = new_active_worker_size;
        std::swap(prev, next);
    }
}

class Solver
{
public:
    Solver(Table<Cell> table, int start_y, int start_x, ItemCounter& counter, const double additional_work_rate);

    std::vector<std::vector<Action>> solve();

    void fill_obstacle(const Table<int>& cluster, Table<Cell>& table, int id);

    std::vector<Point> generate_base_point_list(Worker& worker, int number_cluster);

    int calculate_score();

private:
    std::vector<Worker> worker_list;
    Cluster cluster;
    int worker_size;
    double additional_0;
};

int Solver::calculate_score()
{
    auto& action_list = worker_list[0].get_action_list();
    int index = 1;
    int ret = action_list.size();
    for (int i = 0; i < action_list.size(); i++)
    {
        if (action_list[i].type == ActionType::Cloning)
        {
            const int start_time = i + 1;
            const int score = start_time + worker_list[index].get_action_list().size();
            ret = std::max<int>(ret, score);
            index++;
        }
    }
    return ret;
}

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

    std::vector<int> target_list(cluster_count, 0);
    const int empty_count = worker_list[0].gather(Cell::Empty).size();

    target_list[0] = static_cast<int>(empty_count * (1.0 + additional_0) / (cluster_count + additional_0));
    for (int i = 1; i < cluster_count; i++)
    {
        target_list[i] = static_cast<int>(empty_count / (cluster_count + additional_0));
    }

    for (int i = 0; i < 3000; i++)
    {
        for (int j = 0; j < cluster_count; j++)
        {
            input[j] = candidate[rand(mt)];
        }
        auto clusters = cluster.cluster_count_by_bfs(table, input);
        int eval = 0;
        for (int j = 0; j < cluster_count; j++)
        {
            eval += std::abs(target_list[j] - clusters[j]);
        }
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

Solver::Solver(Table<Cell> table, int start_y, int start_x, ItemCounter& item_counter, double additional_work_rate)
{
    worker_list.resize(1000);
    worker_list[0].init(table, start_y, start_x, item_counter);
    worker_size = 1;
    this->additional_0 = additional_work_rate;
}

std::vector<std::vector<Action>> Solver::solve()
{
    // manipulator / clone しか使えない前提

    const int manipulator_count = worker_list[0].item_counter.get(Cell::ManipulatorExtension);
    for (int i = 0; i < manipulator_count; i++)
    {
        worker_list[0].attach_command();
    }

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

    if (target_list.size() > 0)
    {
        int nearest_index = worker_list[0].select_shortest(target_list);
        if (1 < target_list.size())
        {
            int opponent = nearest_index == 0 ? 1 : 0;
            std::swap(target_list[nearest_index], target_list[opponent]);
        }
        worker_list[0].collect_optimal(target_list);

        const int num_clone = worker_list[0].item_counter.get(Cell::Cloning);

        // Clone
        worker_size = num_clone + 1;

        // Clustering の結果を収集
        if (num_clone > 0)
        {
            std::vector<Point> base_point_list = generate_base_point_list(worker_list[0], num_clone + 1);

            Table<int> clustering_result = cluster.clustering_by_bfs(worker_list[0].get_table(), base_point_list);

            const int nearest_mysterious = worker_list[0].select_shortest(mysterious_point);
            worker_list[0].bfs_move_with_move([&](Point& p) { return p == mysterious_point[nearest_mysterious]; });

            for (int i = 1; i <= num_clone; i++)
            {
                worker_list[i].copy_from(worker_list[0]);
                worker_list[0].clone_command();

                worker_list[i].bfs_move_with_move([&](Point& p) { return p == base_point_list[i]; });
                fill_obstacle(clustering_result, worker_list[i].get_table(), i);
                worker_list[i].divide_and_search();
            }

            worker_list[0].bfs_move_with_move([&](Point& p) { return p == base_point_list[0]; });
            fill_obstacle(clustering_result, worker_list[0].get_table(), 0);
        }
    }
    worker_list[0].divide_and_search();

    std::vector<std::vector<Action>> ret;
    for (int i = 0; i < worker_size; i++)
    {
        ret.push_back(worker_list[i].get_action_list());
    }

    return ret;
}

void print_action(const std::vector<std::vector<xyzworker::Action>>& action_table, std::ostream& out)
{
    bool first = true;
    for (auto& action_list : action_table)
    {
        if (!first)
        {
            out << "#";
        }
        first = false;
        for (auto action : action_list)
        {
            out << action.to_string();
        }
    }
    out << std::endl;
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
} // namespace xyzworker

using namespace xyzworker;

int main(int argc, char* argv[])
{
    std::mt19937_64 mt;
    initialize_hash(xyzworker::place_hash_table, mt);
    initialize_hash(player_hash_table, mt);

    std::string input_filepath(argv[1]);

    int start_y, start_x;
    auto board = load_board(input_filepath, start_y, start_x);

    boardloader::ItemCounter counter;
    counter.reset();

    if (argc == 3)
    {
        std::string additional_booster_filepath(argv[2]);
        auto item_list = boardloader::load_additional_booster(additional_booster_filepath);
        counter += item_list;
    }

    int num_clone = counter.get(boardloader::Cell::Cloning);
    for (auto& row : board)
    {
        for (auto& cell : row)
        {
            if (cell == Cell::Cloning)
            {
                num_clone++;
            }
        }
    }

    std::vector<std::vector<xyzworker::Action>> best_result;
    int best_score = std::numeric_limits<int>::max();

    const int num_try = 1; //num_clone == 0 ? 1 : std::max<int>(5, (400 * 400 * 2) / (board.size() * board[0].size()));

    for (int i = 1; i <= num_try; i++)
    {
        double additional_rate = 0.2; // -0.5 + (static_cast<double>(i) / num_try);
        xyzworker::Solver solver(board, start_y, start_x, counter, additional_rate);
        auto result = solver.solve();
        const int score = solver.calculate_score();
        if (score < best_score)
        {
            best_score = score;
            best_result = result;
        }
    }

    std::cerr << "score = " << best_score << std::endl;

    print_action(best_result, std::cout);
}

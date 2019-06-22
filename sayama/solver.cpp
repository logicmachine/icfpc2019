#include "board_loader.hpp"

#include <algorithm>
#include <queue>
#include <stack>
#include <tuple>

using boardloader::Cell;
using boardloader::load_board;
using boardloader::Point;
using boardloader::print_table;
using boardloader::Table;

namespace xyzworker
{

class Cluster
{
public:
    Table<int> clustering_by_bfs(Table<Cell>& board, std::vector<Point>& point_list);
};

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
    std::vector<Action> solve();

    void init(Table<Cell>& table, int y, int x);

    Worker();
    Worker(const Worker& src);
    void copy_from(const Worker& src);

    Table<Cell>& get_table();

    std::vector<Action>& get_action_list();

    std::vector<Point> gather(Cell cell);

    template <typename FUNCTION>
    bool bfs_move(FUNCTION condition);

    void dfs_with_restart();

    // Command

    void move(Direction dir);

    void clone_command();

private:
    void rotate_clockwise();
    void rotate_counterclockwise();

    void wrap();

    template <typename FUNCTION>
    std::vector<Point> gather_internal(FUNCTION condition);

    void dfs(int cy, int cx);

    bool is_empty(int cy, int cx);

    bool is_inside(int cy, int cx);

    int get_empty_neighrbor_cell(int cy, int cx);

    int get_empty_connected_cell(int cy, int cx, int upperbound);

    std::vector<Point> shortest_path(int ty, int tx);

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
    manipulator_list.emplace_back(0, 1);
    manipulator_list.emplace_back(1, 1);
    manipulator_list.emplace_back(-1, 1);
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
    return gather_internal<>([&](int y, int x) { return table[y][x] == cell; });
}

template <typename FUNCTION>
std::vector<Point> Worker::gather_internal(FUNCTION condition)
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

    for (auto v : src.manipulator_list)
    {
        manipulator_list.push_back(v);
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
    for (const auto& v : manipulator_list)
    {
        const int ny = y + v.y;
        const int nx = x + v.x;
        if (is_inside(ny, nx) && table[ny][nx] != Cell::Obstacle)
        {
            table[ny][nx] = Cell::Occupied;
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

void Worker::dfs_with_restart()
{
    std::vector<std::pair<int, Direction>> selected;

    while (true)
    {
        table[y][x] = Cell::Occupied;
        wrap();

        if (!reset_to_small_region())
        {
            selected.clear();

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
            if (selected.empty())
            {
                if (!bfs_move([&](Point& p) { return is_empty(p.y, p.x); }))
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
    Solver(Table<Cell> table, int start_y, int start_x);

    std::vector<std::vector<Action>> solve();

    void fill_obstacle(const Table<int>& cluster, Table<Cell>& table, int id);

private:
    std::vector<Worker> worker_list;
    Cluster cluster;
};

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

Solver::Solver(Table<Cell> table, int start_y, int start_x)
{
    worker_list.resize(1000);
    worker_list[0].init(table, start_y, start_x);
}

std::vector<std::vector<Action>> Solver::solve()
{
    std::vector<Point> clone_point = worker_list[0].gather(Cell::Cloning);

    std::vector<Point> mysterious_point = worker_list[0].gather(Cell::Mysterious);

    const int num_clone = std::max(0, std::min<int>(clone_point.size(), mysterious_point.size() - 1));

    clone_point.resize(num_clone);
    mysterious_point.resize(num_clone + 1);

    // Clustering の結果を収集

    Table<int> clustering_result = cluster.clustering_by_bfs(worker_list[0].get_table(), mysterious_point);

    // Clone 命令を実行
    // TODO: スケジューリング
    for (int i = 0; i < num_clone; i++)
    {
        // 最短経路で clone 回収
        worker_list[0].bfs_move([&](Point& p) { return p == clone_point[i]; });
    }

    // それぞれの worker に対して、全て map を 自分の id 以外塗りつぶす
    for (int i = 0; i < num_clone; i++)
    {
        worker_list[i].bfs_move([&](Point& p) { return p == mysterious_point[i]; });
        worker_list[i + 1].copy_from(worker_list[i]);
        worker_list[i].clone_command();
        fill_obstacle(clustering_result, worker_list[i].get_table(), i);
        worker_list[i].dfs_with_restart();
    }

    auto& last = worker_list[num_clone];

    // 最後のやつは 自分のエリアを担当
    last.bfs_move([&](Point& p) { return p == mysterious_point.back(); });
    fill_obstacle(clustering_result, last.get_table(), num_clone);
    last.dfs_with_restart();

    std::vector<std::vector<Action>> ret;
    for (int i = 0; i <= num_clone; i++)
    {
        ret.push_back(worker_list[i].get_action_list());
    }

    return ret;
}

} // namespace xyzworker

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

    int start_y, start_x;
    auto board = load_board(input_filepath, start_y, start_x);

    xyzworker::Solver solver(board, start_y, start_x);

    auto result = solver.solve();

    print_action(result, std::cout);
}

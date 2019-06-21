#include "board_loader.hpp"

using boardloader::Cell;
using boardloader::load_board;
using boardloader::Point;
using boardloader::print_table;
using boardloader::Table;

namespace xyzworker
{

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
            return "B" + relative_position.to_string();
        case ActionType::FastWheels:
            return "F";
        case ActionType::Drill:
            return "L";
        }
    }
};

class Worker
{
public:
    Worker(Table<Cell>& table, int y, int x);
    std::vector<Action> solve();

private:
    void rotate_clockwise();
    void rotate_counterclockwise();

    void wrap();
    void dfs(int cy, int cx);

    void move(Direction dir);

    bool is_inside(int cy, int cx);

    Table<Cell> table;
    int y;
    int x;

    int height();
    int width();

    std::vector<Point> manipulator_list;

    std::vector<Action> action_list;

    std::array<int, 4> dx;
    std::array<int, 4> dy;
};

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

int Worker::height()
{
    return table.size();
}

int Worker::width()
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
        if (is_inside(ny, nx) && table[ny][nx] == Cell::Empty)
        {
            table[ny][nx] = Cell::Occupied;
        }
    }
}

void Worker::rotate_clockwise()
{
    for (auto& p : manipulator_list)
    {
        int py = p.y;
        int px = p.x;
        p.y = -p.x;
        p.x = p.y;
    }
}

void Worker::rotate_counterclockwise()
{
    for (auto& p : manipulator_list)
    {
        int py = p.y;
        int px = p.x;
        p.y = p.x;
        p.x = -p.y;
    }
}

Worker::Worker(Table<Cell>& table, int y, int x)
    : table(table)
    , y(y)
    , x(x)
{
    // Up, Left, Down, Right
    dy = { 1, 0, -1, 0 };
    dx = { 0, -1, 0, 1 };

    manipulator_list.emplace_back(0, 0);
    manipulator_list.emplace_back(1, 0);
    manipulator_list.emplace_back(1, 1);
    manipulator_list.emplace_back(1, -1);
}

void Worker::dfs(int cy, int cx)
{
    table[cy][cx] = Cell::Occupied;

    for (int i = 0; i < 4; i++)
    {
        const int ny = cy + dy[i];
        const int nx = cx + dx[i];
        if (is_inside(ny, nx) && table[ny][nx] == Cell::Empty)
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
    dfs(y, x);

    return action_list;
}
} // namespace xyzworker

void save_action(const std::vector<xyzworker::Action>& action_list, const std::string& filepath)
{
    std::ofstream fout;
    fout.open(filepath);

    if (!fout.is_open())
    {
        std::cerr << "cannot open file @ save_action." << std::endl;
        return;
    }

    for (auto action : action_list)
    {
        fout << action.to_string();
    }
    fout.close();
}

int main(int argc, char* argv[])
{
    std::string input_filepath(argv[1]);
    std::string output_filepath(argv[2]);

    int start_y, start_x;
    auto board = load_board(input_filepath, start_y, start_x);

    xyzworker::Worker worker(board, start_y, start_x);

    auto result = worker.solve();

    save_action(result, output_filepath);
}
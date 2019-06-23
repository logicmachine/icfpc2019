//#include "board_loader.hpp"

#include "bits/stdc++.h"

#include <algorithm>
#include <queue>
#include <stack>

#include "../ccl.hpp"
#include "../../ikeda/ikeda.hpp"
//#include "ikeda.hpp"

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

    bool is_intersect_line_seg(double y1, double x1, double y2, double x2, double y3, double x3, double y4, double x4);
    bool is_intersect(double y1, double x1, double y2, double x2, double y3, double x3, double y4, double x4);
    bool is_reachable(int src_y, int src_x, int dst_y, int dst_x);

    void wrap();
    bool bfs();
    void dfs(int cy, int cx);
    void dfs_with_restart();

    void move(Direction dir);

    Direction get_move_direction(char a);
    void pickup_manipulators();
    void get_ccl_data(const std::vector<std::vector<Cell>>& table, std::vector<int>& data, int& W);
    void calc_ccl(std::vector<int>& data, const int W, std::vector<int>& result, vector<int>& spaces);
    void check_ccl(const std::vector<std::vector<Cell>>& table, vector<int>& result, vector<int>& spaces);
    Point nearest_cc(int y, int x);
    Point largest_cc(int y, int x);

    bool is_inside(int cy, int cx);

    int get_empty_neighrbor_cell(int cy, int cx);

    Table<Cell> table;
    int y;
    int x;

    int height();
    int width();

    std::vector<Point> manipulator_list;

    std::vector<Action> action_list;

    std::array<int, 4> dx;
    std::array<int, 4> dy;

	nf_ccl::CCL ccl;
    std::vector<int> ccl_data;
    std::vector<int> ccl_spaces;
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

bool Worker::is_intersect_line_seg(double y1, double x1, double y2, double x2, double y3, double x3, double y4, double x4)
{
    double ta=(x1-x2)*(y3-y1)+(y1-y2)*(x1-x3);
    double tb=(x1-x2)*(y4-y1)+(y1-y2)*(x1-x4);
    if( ta*tb<0 ) return true;
    else return false;
}

bool Worker::is_intersect(double y1, double x1, double y2, double x2, double y3, double x3, double y4, double x4)
{
    if (is_intersect_line_seg(y1, x1, y2, x2, y3, x3, y4, x4) and is_intersect_line_seg(y3, x3, y4, x4, y1, x1, y2, x2)) {
        return true;
    } else {
        return false;
    }
}

bool Worker::is_reachable(int src_y, int src_x, int dst_y, int dst_x)
{
    int dy[] = {0, 0, 1, 1, 0};
    int dx[] = {0, 1, 1, 0, 0};
    for (int i = std::min(src_y, dst_y); i < std::max(src_y, dst_y); i++) {
        for (int j = std::min(src_x, dst_x); j < std::max(src_x, dst_x); j++) {
            if (table[i][j] != Cell::Obstacle) continue;
            for (int k = 0; k < 4; k++) {
                if (is_intersect(src_y+0.5, src_x+0.5, dst_y+0.5, dst_x+0.5, i+dy[k], j+dx[k], i+dy[k+1], j+dx[k+1])) {
                    return false;
                }
            }
        }
    }
    return true;
}
    
void Worker::wrap()
{
    // TODO: 当たり判定を入れる
    int order = 0; /// TODO:
    for (const auto& v : manipulator_list)
    {
        const int ny = y + v.y;
        const int nx = x + v.x;
        if (is_inside(ny, nx) && table[ny][nx] == Cell::Empty)
        //if (is_inside(ny, nx) && (table[ny][nx] != Cell::Obstacle && table[ny][nx] != Cell::Occupied))
        {
            /*
            if (is_reachable(y, x, ny, nx)) {
                table[ny][nx] = Cell::Occupied;
            }
            */
            table[ny][nx] = Cell::Occupied;
        } else {
            if (is_inside(ny, nx) && order > 2 && table[ny][nx] == Cell::Obstacle) break;
        }
        order++; /// TODO:
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

void Worker::get_ccl_data(const std::vector<std::vector<Cell>>& table, std::vector<int>& data, int& W)
{
    W = table[0].size();
    //vector<int> data;
    for (const auto& xs : table) {
        for (const auto& x : xs) {
            data.push_back((x == Cell::Obstacle || x == Cell::Occupied) ? 1 : 0);
        }
    }
}

void Worker::calc_ccl(std::vector<int>& data, const int W, std::vector<int>& result, vector<int>& spaces)
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

//void Worker::check_ccl(const std::vector<std::vector<Cell>>& table, std::vector<int>& data, int& W, vector<int>& result, vector<int>& spaces)
void Worker::check_ccl(const std::vector<std::vector<Cell>>& table, vector<int>& result, vector<int>& spaces)
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
    ///std::cerr << "number of conncted components: " << spaces.size() << std::endl;

}

Worker::Worker(Table<Cell>& table, int y, int x)
    : table(table)
    , y(y)
    , x(x)
{
    // Up, Left, Down, Right
    dy = { 1, 0, -1, 0 };
    dx = { 0, -1, 0, 1 };
    // Left, Up, Down, Right
    //dy = { 0, 1, -1, 0 };
    //dx = { -1, 0, 0, 1 };

    /*
    manipulator_list.emplace_back(0, 0);
    manipulator_list.emplace_back(1, 0);
    manipulator_list.emplace_back(1, 1);
    manipulator_list.emplace_back(1, -1);
    */
    manipulator_list.emplace_back(0, 0);
    manipulator_list.emplace_back(1, 1);
    manipulator_list.emplace_back(-1, 1);
    manipulator_list.emplace_back(0, 1);

    check_ccl(table, ccl_data, ccl_spaces);
}

bool Worker::bfs()
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

        //if (table[p.y][p.x] == Cell::Empty)
        if (table[p.y][p.x] != Cell::Obstacle && table[p.y][p.x] != Cell::Occupied)
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

int Worker::get_empty_neighrbor_cell(int cy, int cx)
{
    int count = 0;
    for (int i = 0; i < 4; i++)
    {
        const int ny = cy + dy[i];
        const int nx = cx + dx[i];
        if (is_inside(ny, nx) && table[ny][nx] != Cell::Occupied && table[ny][nx] != Cell::Obstacle)
        {
            count++;
        }
    }
    return count;
}

//Point Worker::nearest_cc(const std::vector<std::vector<Cell>>& table)
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
    for (const auto& space : ccl_spaces) {
        area.clear();
        for (int i = 0; i < ccl_data.size(); i++) {
            if (ccl_data[i] == space) {
                //area.push_back(Point(H - i / W - 1, i % W));
                area.push_back(Point(i / W, i % W));
            }
        }
        std::shuffle(area.begin(), area.end(), engine);
#if 0
        Point max_x_pos, max_y_pos, min_x_pos, min_y_pos;
        unsigned int max_x = 0;
        unsigned int max_y = 0;
        unsigned int min_x = -1;
        unsigned int min_y = -1;
        for (const auto& pos : area) {
            if (max_x < pos.x) {
                max_x = pos.x;
                max_x_pos = pos;
            }
            if (max_y < pos.y) {
                max_y = pos.y;
                max_y_pos = pos;
            }
            if (min_x > pos.x) {
                min_x = pos.x;
                min_x_pos = pos;
            }
            if (min_y > pos.y) {
                min_y = pos.x;
                min_y_pos = pos;
            }
        }
        std::vector<Point> candidates;
        candidates.push_back(max_x_pos);
        candidates.push_back(max_y_pos);
        candidates.push_back(min_x_pos);
        candidates.push_back(min_y_pos);
#endif
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
        //if (next_dist > longest_dist) {
        if (next_dist > nearest_dist) {
            //next_dist = longest_dist;
            //next_pos = longest_pos;
            next_dist = nearest_dist;
            next_pos = nearest_pos;
        }
    }

    return next_pos;
}

Point Worker::largest_cc(int y, int x)
{
    std::random_device seed_gen;
    std::mt19937 engine(seed_gen());

    std::vector<Point> area;
    int H = table.size();
    int W = table[0].size();
    //unsigned int next_dist = -1;
    Point next_pos({-1, -1});
    unsigned int max_ccl_size = 0;
    unsigned int min_ccl_size = -1;

    for (const auto& space : ccl_spaces) {
        int ccl_size = count(ccl_data.begin(), ccl_data.end(), space);
#if 0
        if (max_ccl_size < ccl_size) {
            max_ccl_size = ccl_size;
#else
        if (min_ccl_size > ccl_size) {
            min_ccl_size = ccl_size;
#endif
            area.clear();
            for (int i = 0; i < ccl_data.size(); i++) {
                if (ccl_data[i] == space) {
                    //area.push_back(Point(H - i / W - 1, i % W));
                    area.push_back(Point(i / W, i % W));
                }
            }
            std::shuffle(area.begin(), area.end(), engine);
            next_pos = area.front();
        }
    }

    return next_pos;
}

void Worker::dfs_with_restart()
{
    std::random_device seed_gen;
    std::mt19937 engine(seed_gen());
    std::uniform_real_distribution<> dist(0.0, 1.0);

    std::vector<std::pair<int, Direction>> selected;

    while (true)
    {
        if (table[y][x] == Cell::ManipulatorExtension) {
            int num_manip = manipulator_list.size();
            manipulator_list.emplace_back(0, num_manip - 2);
            //action_list.emplace_back(ActionType::Attatch, Point(0, num_manip - 2));
            action_list.emplace_back(ActionType::Attatch, Point(num_manip - 2, 0)); /// TODO:
        }

        table[y][x] = Cell::Occupied;

#if 0
        if (x+1 < table[0].size()) {
            if (table[y][x+1] == Cell::Empty) table[y][x+1] = Cell::Occupied;
            if (y+1 < table.size() && table[y+1][x+1] == Cell::Empty) table[y+1][x+1] = Cell::Occupied;
            if (y-1 >= 0 && table[y-1][x+1] == Cell::Empty) table[y-1][x+1] = Cell::Occupied;
        }
#else
        wrap();
#endif

        //check_ccl(table, ccl_data, ccl_spaces); ////// debug

        selected.clear();

        for (int i = 0; i < 4; i++)
        {
            const int ny = y + dy[i];
            const int nx = x + dx[i];
            if (is_inside(ny, nx) && table[ny][nx] == Cell::Empty)
            {
                const Direction dir = static_cast<Direction>(i);
                //selected.emplace_back(-get_empty_neighrbor_cell(ny, nx), dir);
                selected.emplace_back(dy[i], dir); /// debug
            }
        }
        if (selected.empty())
        {
//#if 1
            if (dist(engine) < 0.3) {
            check_ccl(table, ccl_data, ccl_spaces); ////// debug
            Point next_pos(nearest_cc(y, x));
            //Point next_pos(largest_cc(y, x));
            ///std::cerr << "next_pos: " << next_pos.x << ", " << next_pos.y << std::endl;
            if (next_pos.x == -1) break;
            std::string move_string = ikeda::move(table, {y, x}, {next_pos.y, next_pos.x});
            for (char a : move_string) {
                move(get_move_direction(a));
                wrap();
            }
            } else {
//#else
            if (!bfs())
            {
                break;
            }
//#endif
            }
        }
        else
        {
            sort(selected.begin(), selected.end());
            //sort(selected.begin(), selected.end(), std::greater<std::pair<int, Direction>>());
            move(selected[0].second);
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
        //if (is_inside(ny, nx) && table[ny][nx] == Cell::Empty)
        if (is_inside(ny, nx) && (table[ny][nx] != Cell::Obstacle && table[ny][nx] != Cell::Occupied))
        {
            const Direction dir = static_cast<Direction>(i);
            move(dir);
            dfs(ny, nx);
            move(inverse(dir));
        }
    }
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

void Worker::pickup_manipulators()
{
    std::vector<Point> manip_pos_list;
    for (int yy = 0; yy < table.size(); yy++) {
        for (int xx = 0; xx < table[0].size(); xx++) {
            if (table[yy][xx] == Cell::ManipulatorExtension) {
                manip_pos_list.push_back(Point(yy, xx));
            }
        }
    }
    
    ///std::cerr << manip_pos_list.size() << std::endl; ///// debug
    
    //int res = ikeda::calc_distance( robot.field, {order.back().first, order.back().second}, {i, j} );
    int cur_x = x;
    int cur_y = y;
    std::string move_string("");
    while (!manip_pos_list.empty()) {
        Point shortest_pos;
        unsigned int shortest_length = -1U;
        for (auto pos : manip_pos_list) {
            int res = ikeda::calc_distance(table, {cur_y, cur_x}, {pos.y, pos.x});
            if (shortest_length > res) {
                shortest_length = res;
                shortest_pos = pos;
            }
        }
        ///std::cerr << shortest_length << std::endl; ////// debug
        move_string += ikeda::move(table, {cur_y, cur_x}, {shortest_pos.y, shortest_pos.x});
        cur_y = shortest_pos.y;
        cur_x = shortest_pos.x;
        manip_pos_list.erase(std::remove(manip_pos_list.begin(), manip_pos_list.end(), shortest_pos), manip_pos_list.end());
    }

    for (char a : move_string) {
        wrap();
        move(get_move_direction(a));
    }
}

std::vector<Action> Worker::solve()
{
    pickup_manipulators();

    dfs_with_restart();
    // dfs(y, x);

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

void output_action(const std::vector<xyzworker::Action>& action_list)
{
    for (auto action : action_list)
    {
        std::cout << action.to_string();
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
    std::string input_filepath(argv[1]);
    //std::string output_filepath(argv[2]);

    int start_y, start_x;
    auto board = load_board(input_filepath, start_y, start_x);

    //for (int i = 0; i < 10; i++) {
    xyzworker::Worker worker(board, start_y, start_x);

    auto result = worker.solve();

    std::cerr << "Size: " << result.size() << std::endl; ///// debug
    
    //save_action(result, output_filepath);
    output_action(result);
    //}
}

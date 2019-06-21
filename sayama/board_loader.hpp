#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace boardloader
{
enum Cell
{
    Empty = '.',
    Occupied = '@',
    Obstacle = '#',
    ManipulatorExtension = 'B',
    FastWheels = 'F',
    Drill = 'L',
    Mysterious = 'X',
};

Cell to_cell(const char ch)
{
    switch (ch)
    {
    case 'B':
        return Cell::ManipulatorExtension;
    case 'F':
        return Cell::FastWheels;
    case 'L':
        return Cell::Drill;
    case 'X':
        return Cell::Mysterious;
    }
    assert(false);
}

template <typename T>
using Table = std::vector<std::vector<T>>;

void print_table(Table<Cell>& table, int y, int x)
{
    for (int i = table.size() - 1; i >= 0; i--)
    {
        for (int j = 0; j < table[i].size(); j++)
        {
            std::cout << (i == y && j == x ? 'P' : static_cast<char>(table[i][j]));
        }
        std::cout << std::endl;
    }
}

struct Point
{
    int y;
    int x;

    Point(int y, int x)
        : y(y)
        , x(x)
    {
    }

    Point() {}

    void min(const Point& p)
    {
        y = std::min(y, p.y);
        x = std::min(x, p.x);
    }

    void max(const Point& p)
    {
        y = std::max(y, p.y);
        x = std::max(x, p.x);
    }

    Point operator+(const Point p)
    {
        return Point(y + p.y, x + p.x);
    }

    Point operator-(const Point p)
    {
        return Point(y - p.y, x - p.x);
    }

    void operator+=(const Point p)
    {
        y += p.y;
        x += p.x;
    }

    void operator-=(const Point p)
    {
        y -= p.y;
        x -= p.x;
    }

    Point operator*(const int v)
    {
        y *= v;
        x *= v;
        return *this;
    }

    bool operator==(const Point& p)
    {
        return y == p.y && x == p.x;
    }

    bool operator!=(const Point& p)
    {
        return !(*this == p);
    }
};

Point get_direction(const Point& from, const Point& to)
{
    const int dy = to.y == from.y ? 0 : (to.y - from.y) / abs(to.y - from.y);
    const int dx = to.x == from.x ? 0 : (to.x - from.x) / abs(to.x - from.x);

    return Point(dy, dx);
}

std::ostream& operator<<(std::ostream& out, const Point& p)
{
    out << "(" << p.y << ", " << p.x << ")";
    return out;
}

std::string read_all(const std::string& filepath)
{
    std::ifstream fin;
    fin.open(filepath);

    if (fin.bad())
    {
        std::cerr << "fail to open file..." << std::endl;
        return "";
    }
    std::string str((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    fin.close();

    return str;
}

std::vector<std::string> split(const std::string str, const char delimiter)
{
    std::vector<std::string> ret;

    int begin = 0;
    int index = 0;

    while (index < str.size())
    {
        while (index < str.size() && str[index] != delimiter)
        {
            index++;
        }
        ret.push_back(str.substr(begin, index - begin));
        index++;
        begin = index;
    }

    return ret;
}

bool is_number(const char ch)
{
    return '0' <= ch && ch <= '9';
}

int parse_int(const std::string& str, int& sindex)
{
    int ret = 0;
    while (sindex < str.size() && is_number(str[sindex]))
    {
        ret *= 10;
        ret += str[sindex] - '0';
        sindex++;
    }
    return ret;
}

Point parse_point(const std::string str, int& sindex)
{
    // (
    sindex++;
    int x = parse_int(str, sindex);
    // ,
    sindex++;
    int y = parse_int(str, sindex);
    // )
    sindex++;
    return Point(y, x);
}

Point parse_start(const std::string str)
{
    int sindex = 0;
    return parse_point(str, sindex);
}

std::vector<Point> parse_maps(const std::string str, int& sindex)
{
    std::vector<Point> ret;

    while (sindex < str.size())
    {
        ret.push_back(parse_point(str, sindex));
        // ,
        if (str[sindex] == ';')
        {
            break;
        }
        sindex++;
    }
    return ret;
}

std::vector<std::vector<Point>> parse_obstacles(const std::string str)
{
    int sindex = 0;
    std::vector<std::vector<Point>> ret;
    while (sindex < str.size())
    {
        ret.push_back(parse_maps(str, sindex));
        sindex++;
    }
    return ret;
}

std::pair<Point, Point> calculate_map_edge(const std::vector<Point>& map)
{
    Point max(std::numeric_limits<int>::min(), std::numeric_limits<int>::min());
    Point min(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());

    for (auto& p : map)
    {
        max.max(p);
        min.min(p);
    }
    return std::make_pair(min, max);
}

void fill(Table<Cell>& table, const std::vector<Point>& border_list, Cell to)
{
    std::vector<Point> double_border_list;
    for (auto v : border_list)
    {
        double_border_list.push_back(Point(v.y * 2, v.x * 2));
    }

    const int twice_height = (table.size() + 1) * 2;
    const int twice_width = (table[0].size() + 1) * 2;

    Table<bool> visited(twice_height, std::vector<bool>(twice_width, false));

    // mark border
    Point p = double_border_list[0];

    for (int i = 0; i < double_border_list.size() - 1; i++)
    {
        Point dir = get_direction(double_border_list[i], double_border_list[i + 1]);
        while (p != double_border_list[i + 1])
        {
            visited[p.y][p.x] = true;
            p += dir;
        }
    }

    Point dir = get_direction(double_border_list.back(), double_border_list.front());
    while (p != double_border_list[0])
    {
        visited[p.y][p.x] = true;
        p += dir;
    }

    for (int y2 = 1; y2 < twice_height; y2 += 2)
    {
        bool flip = false;
        for (int x2 = 1; x2 < twice_width; x2 += 2)
        {
            if (visited[y2][x2 - 1])
            {
                flip = !flip;
            }
            if (flip)
            {
                table[y2 / 2][x2 / 2] = to;
            }
        }
    }
}

std::pair<Cell, Point> parse_single_booster(const std::string str, int& sindex)
{
    char border_code = str[sindex];
    sindex++;
    auto p = parse_point(str, sindex);
    return std::make_pair(to_cell(border_code), p);
}

std::map<Cell, std::vector<Point>> parse_booster(const std::string str)
{
    std::map<Cell, std::vector<Point>> m;

    int sindex = 0;
    while (sindex < str.size())
    {
        Cell cell;
        Point p;
        std::tie(cell, p) = parse_single_booster(str, sindex);
        sindex++;
        m[cell].push_back(p);
    }

    return m;
}

Table<Cell> load(const std::string& filepath, int& start_y, int& start_x)
{
    auto str = read_all(filepath);
    auto splitted = split(str, '#');

    int sindex = 0;
    auto maps = parse_maps(splitted[0], sindex);

    auto start_point = parse_start(splitted[1]);
    start_y = start_point.y;
    start_x = start_point.x;

    auto obstacles = parse_obstacles(splitted[2]);

    auto booster = parse_booster(splitted[3]);

    Point min_point, max_point;
    std::tie(min_point, max_point) = calculate_map_edge(maps);

    Table<Cell> table;
    table.resize(max_point.y - min_point.y);
    for (auto& row : table)
    {
        row.resize(max_point.x - min_point.x, Cell::Obstacle);
    }

    fill(table, maps, Cell::Empty);

    for (auto& obstacle : obstacles)
    {
        fill(table, obstacle, Cell::Obstacle);
    }

    for (auto cell : booster)
    {
        for (auto p : booster[cell.first])
        {
            table[p.y][p.x] = cell.first;
        }
    }
    return table;
}
} // namespace board

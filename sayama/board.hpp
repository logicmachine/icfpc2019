#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace boardloader
{
enum Cell
{
    Empty,
    Occupied,
    Obstacle,
    ManipulatorExtension,
    FastWheels,
    Drill,
    Mysterious,
};

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

std::pair<int, int> parse_point(const std::string str, int& sindex)
{
    // (
    sindex++;
    int y = parse_int(str, sindex);
    // ,
    sindex++;
    int x = parse_int(str, sindex);
    // )
    sindex++;
    return std::make_pair(y, x);
}

std::pair<int, int> parse_start(const std::string str)
{
    int sindex = 0;
    return parse_point(str, sindex);
}

std::vector<std::pair<int, int>> parse_maps(const std::string str, int& sindex)
{
    std::vector<std::pair<int, int>> ret;

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

std::vector<std::vector<std::pair<int, int>>> parse_obstacles(const std::string str)
{
    int sindex = 0;
    std::vector<std::vector<std::pair<int, int>>> ret;
    while (sindex < str.size())
    {
        ret.push_back(parse_maps(str, sindex));
        sindex++;
    }
    return ret;
}

std::vector<std::vector<Cell>>
load(const std::string& filepath, int& start_y, int& start_x)
{
    auto str = read_all(filepath);
    auto splitted = split(str, '#');

    int sindex = 0;
    auto maps = parse_maps(splitted[0], sindex);

    auto yx = parse_start(splitted[1]);
    start_y = yx.first;
    start_x = yx.second;

    auto obstacles = parse_obstacles(splitted[2]);

    return std::vector<std::vector<Cell>>();
}
} // namespace board

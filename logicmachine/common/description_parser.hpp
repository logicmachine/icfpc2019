#ifndef COMMON_DESCRIPTION_PARSER_HPP
#define COMMON_DESCRIPTION_PARSER_HPP

#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <cctype>
#include <cassert>

#include "vec2.hpp"
#include "matrix.hpp"
#include "parser_common.hpp"


namespace detail {

static std::vector<Vec2> parse_map(const char *s, size_t& i){
	std::vector<Vec2> result;
	while(s[i] == '('){
		result.push_back(parse_point(s, i));
		if(s[i] != ','){ break; }
		++i;  // ignore ','
	}
	return result;
}

static std::vector<std::vector<Vec2>> parse_obstacles(const char *s, size_t& i){
	std::vector<std::vector<Vec2>> result;
	while(s[i] == '('){
		result.push_back(parse_map(s, i));
		if(s[i] != ';'){ break; }
		++i;  // ignore ';'
	}
	return result;
}

static std::pair<char, Vec2> parse_booster_location(const char *s, size_t& i){
	const char code = s[i++];
	const auto point = parse_point(s, i);
	return std::make_pair(code, point);
}

static std::vector<std::pair<char, Vec2>> parse_boosters(const char *s, size_t& i){
	std::vector<std::pair<char, Vec2>> result;
	while(isalpha(s[i])){
		result.push_back(parse_booster_location(s, i));
		if(s[i] != ';'){ break; }
		++i;  // ignore ';'
	}
	return result;
}

static void update_differential(Matrix<int>& differential, const std::vector<Vec2>& poly){
	const size_t n = poly.size();
	for(size_t i = 0; i < n; ++i){
		const auto& a = poly[i];
		const auto& b = poly[(i + 1) % n];
		const auto lo = std::min(a.y, b.y), hi = std::max(a.y, b.y);
		for(int k = lo; k < hi; ++k){ differential(k, a.x) ^= 1; }
	}
}

}

static std::string parse_task_description(const std::string& raw){
	const char *s = raw.c_str();
	size_t index = 0;
	const auto map = detail::parse_map(s, index);
	assert(s[index] == '#'); ++index;
	const auto point = detail::parse_point(s, index);
	assert(s[index] == '#'); ++index;
	const auto obstacles = detail::parse_obstacles(s, index);
	assert(s[index] == '#'); ++index;
	const auto boosters = detail::parse_boosters(s, index);

	int rows = 0, cols = 0;
	for(const auto& p : map){
		rows = std::max(rows, p.y);
		cols = std::max(cols, p.x);
	}
	Matrix<int> differential(rows + 1, cols + 1);
	detail::update_differential(differential, map);
	for(const auto& obstacle : obstacles){
		detail::update_differential(differential, obstacle);
	}

	std::vector<std::string> grid(rows, std::string(cols, '.'));
	for(int i = 0; i < rows; ++i){
		int state = 0;
		for(int j = 0; j < cols; ++j){
			state ^= differential(i, j);
			if(!state){ grid[i][j] = '#'; }
		}
	}
	grid[point.y][point.x] = 'P';
	for(const auto& b : boosters){
		const auto& p = b.second;
		grid[p.y][p.x] = b.first;
	}

	std::ostringstream oss;
	for(const auto& line : grid){ oss << line << std::endl; }
	return oss.str();
}

#endif

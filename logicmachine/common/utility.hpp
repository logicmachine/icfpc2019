#ifndef COMMON_UTILITY_HPP
#define COMMON_UTILITY_HPP

#include <fstream>
#include <sstream>
#include <string>
#include <array>

#include "vec2.hpp"
#include "matrix.hpp"

static const std::array<Vec2, 4> NEIGHBORS = {
	Vec2(1, 0), Vec2(0, 1), Vec2(-1, 0), Vec2(0, -1)
};
static const int DIR_RIGHT = 0;
static const int DIR_UP    = 1;
static const int DIR_LEFT  = 2;
static const int DIR_DOWN  = 3;

static const int ROTATE_LEFT  =  1;
static const int ROTATE_RIGHT = -1;

inline bool between(int a, int b, int c){
	return a <= b && b < c;
}

template <typename T>
inline bool in_range(const Matrix<T>& mat, const Vec2& c){
	return between(0, c.y, mat.rows()) && between(0, c.x, mat.cols());
}


static std::string read_file(const std::string& filename){
	std::ifstream ifs(filename);
	std::ostringstream oss;
	std::string line;
	while(std::getline(ifs, line)){ oss << line << std::endl; }
	return oss.str();
}

#endif

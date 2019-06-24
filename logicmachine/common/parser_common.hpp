#ifndef COMMON_PARSER_COMMON_HPP
#define COMMON_PARSER_COMMON_HPP

#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <cctype>
#include <cassert>

#include "vec2.hpp"
#include "matrix.hpp"


namespace detail {

static int parse_int(const char *s, size_t& i){
	int value = 0, sign = 1;
	if(s[i] == '-'){
		sign = -1;
		++i;
	}
	while(isdigit(s[i])){
		value = value * 10 + (s[i] - '0');
		++i;
	}
	return sign * value;
}

static Vec2 parse_point(const char *s, size_t& i){
	assert(s[i] == '('); ++i;
	const int x = parse_int(s, i);
	assert(s[i] == ','); ++i;
	const int y = parse_int(s, i);
	assert(s[i] == ')'); ++i;
	return Vec2(x, y);
}

}

#endif

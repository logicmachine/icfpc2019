#ifndef COMMON_VEC2_HPP
#define COMMON_VEC2_HPP

#include <iostream>
#include <functional>


struct Vec2 {
	int x;
	int y;

	Vec2() : x(0), y(0) { }
	Vec2(int x, int y) : x(x), y(y) { }

	Vec2 operator+(const Vec2 &p) const { return Vec2(x + p.x, y + p.y); }
	Vec2 &operator+=(const Vec2 &p){ return *this = *this + p; }
	Vec2 operator-(const Vec2 &p) const { return Vec2(x - p.x, y - p.y); }
	Vec2 &operator-=(const Vec2 &p){ return *this = *this - p; }
	Vec2 operator*(int s) const { return Vec2(x * s, y * s); }
	Vec2 &operator*=(int s){ return *this = *this * s; }
	Vec2 operator*(const Vec2 &p) const {
		return Vec2(x * p.x - y * p.y, x * p.y + y * p.x);
	}
	Vec2 &operator*=(const Vec2 &p){ return *this = *this * p; }
	Vec2 operator/(int s) const { return Vec2(x / s, y / s); }
	Vec2 &operator/=(int s){ return *this = *this / s; }

	bool operator==(const Vec2 &p) const { return x == p.x && y == p.y; }
	bool operator!=(const Vec2 &p) const { return x != p.x || y != p.y; }
	bool operator<(const Vec2 &p) const {
		return (x == p.x) ? (y < p.y) : (x < p.x);
	}

	int norm() const { return x * x + y * y; }
	Vec2 ortho() const { return Vec2(-y, x); }
};

inline Vec2 operator*(int s, const Vec2 &p){ return p * s; }

inline int cross(const Vec2 &a, const Vec2 &b){
	return a.x * b.y - a.y * b.x;
}

inline int dot(const Vec2 &a, const Vec2 &b){
	return a.x * b.x + a.y * b.y;
}

inline int ccw(const Vec2 &a, const Vec2 &b, const Vec2 &c){
	Vec2 d = b - a, e = c - a;
	if(cross(d, e) > 0){ return  1; }
	if(cross(d, e) < 0){ return -1; }
	return 0;
}

namespace std {
template <> struct hash<Vec2> {
	size_t operator()(const Vec2& v) const {
		return static_cast<size_t>(v.y) * 1000000007 + v.x;
	}
};
}

inline std::ostream& operator<<(std::ostream& os, const Vec2& v){
	os << "(" << v.x << ", " << v.y << ")";
	return os;
}

#endif

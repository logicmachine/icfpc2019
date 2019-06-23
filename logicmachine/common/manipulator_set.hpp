#ifndef COMMON_MANIPULATOR_SET_HPP
#define COMMON_MANIPULATOR_SET_HPP

#include "vec2.hpp"


class ManipulatorSet {

private:
	std::vector<Vec2> m_offsets;

public:
	ManipulatorSet()
		: m_offsets(1, Vec2(0, 0))
	{ }

	bool insert(const Vec2& offset){
		// TODO validation
		m_offsets.push_back(offset);
		return true;
	}

	const std::vector<Vec2>& offsets() const { return m_offsets; }

	void rotate_right(){
		for(auto& offset : m_offsets){ offset = Vec2(offset.y, -offset.x); }
	}

	void rotate_left(){
		for(auto& offset : m_offsets){ offset = Vec2(-offset.y, offset.x); }
	}

	static ManipulatorSet make_initial_set(){
		ManipulatorSet ms;
		ms.insert(Vec2(1, -1));
		ms.insert(Vec2(1,  0));
		ms.insert(Vec2(1,  1));
		return ms;
	}

};

#endif

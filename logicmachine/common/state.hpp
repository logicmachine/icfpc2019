#ifndef COMMON_STATE_HPP
#define COMMON_STATE_HPP

#include <array>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <cassert>
#include <cstdint>

#include <boost/optional.hpp>

#include "types.hpp"
#include "matrix.hpp"
#include "command.hpp"
#include "manipulator_set.hpp"
#include "utility.hpp"


struct Wrapper {
	Vec2 position;
	ManipulatorSet manipulators;
	int wheel_remains = 0;
	int drill_remains = 0;
};

struct Boosters {
	int manipulators = 0;
	int fast_wheels  = 0;
	int drills       = 0;
	int teleports    = 0;
	int clones       = 0;
};

class State {

public:
	class UndoBuffer {
	private:
		size_t wrapper_id;
		Wrapper wrapper;
		Boosters boosters;
		std::unordered_map<Vec2, CellKind> replaces;
		std::unordered_set<Vec2> wrapped;
	public:
		UndoBuffer()
			: wrapper_id(0)
			, wrapper()
			, boosters()
			, replaces()
			, wrapped()
		{ }
		UndoBuffer(size_t wid, const Wrapper& w, const Boosters& b)
			: wrapper_id(wid)
			, wrapper(w)
			, boosters(b)
			, replaces()
			, wrapped()
		{ }
		void replace(State& s, size_t i, size_t j, CellKind new_kind){
			const Vec2 p(j, i);
			if(replaces.find(p) != replaces.end()){
				s.m_field(i, j) = new_kind;
			}else if(s.m_field(i, j) != new_kind){
				replaces.emplace(p, s.m_field(i, j));
				s.m_field(i, j) = new_kind;
			}
		}
		void wrap(State& s, size_t i, size_t j){
			if(s.m_wrapped(i, j)){ return; }
			s.m_wrapped(i, j) = true;
			wrapped.emplace(j, i);
		}
		void apply(State& s) const {
			if(s.m_boosters.clones < boosters.clones){
				s.m_wrappers.pop_back();
			}
			for(const auto& r : replaces){
				const auto p = r.first;
				s.m_field(p.y, p.x) = r.second;
			}
			for(const auto& p : wrapped){
				s.m_wrapped(p.y, p.x) = false;
			}
			s.m_wrappers[wrapper_id] = wrapper;
			s.m_boosters = boosters;
		}
	};

private:
	Matrix<CellKind> m_field;
	Matrix<uint8_t> m_wrapped;
	std::vector<Wrapper> m_wrappers;
	Boosters m_boosters;

	template <typename F>
	void enumerate_reaches(const Vec2& p, const ManipulatorSet& ms, F f){
		for(const auto& d : ms.offsets()){
			const auto target = p + d;
			if(!in_range(m_field, target)){ continue; }
			const int min_y = std::min(p.y, p.y + d.y), max_y = std::max(p.y, p.y + d.y);
			const int min_x = std::min(p.x, p.x + d.x), max_x = std::max(p.x, p.x + d.x);
			const auto a = p      * 2 + Vec2(1, 1);
			const auto b = target * 2 + Vec2(1, 1);
			bool accept = true;
			for(int y = min_y; accept && y <= max_y; ++y){
				for(int x = min_x; accept && x <= max_x; ++x){
					if(m_field(y, x) != CellKind::OBSTACLE){ continue; }
					const std::array<Vec2, 4> cs = {
						Vec2(y + 0, x + 0) * 2,  Vec2(y + 0, x + 1) * 2,
						Vec2(y + 1, x + 0) * 2,  Vec2(y + 1, x + 1) * 2
					};
					int acc_ccw = 0;
					for(const auto& c : cs){
						const int k = ccw(a, b, c);
						if(k == 0){ continue; }
						if(acc_ccw == 0){
							acc_ccw = k;
						}else if(acc_ccw != k){
							accept = false;
							break;
						}
					}
				}
			}
			if(accept){ f(target); }
		}
	}

public:
	State()
		: m_field()
		, m_wrapped()
		, m_wrappers()
		, m_boosters()
	{ }

	size_t rows() const { return m_field.rows(); }
	size_t cols() const { return m_field.cols(); }

	const Matrix<CellKind>& field() const { return m_field; }
	CellKind field(size_t i, size_t j) const { return m_field(i, j); }

	const Matrix<uint8_t>& wrapped() const { return m_wrapped; }
	uint8_t wrapped(size_t i, size_t j) const { return m_wrapped(i, j); }

	size_t num_wrappers() const { return m_wrappers.size(); }
	const std::vector<Wrapper>& wrappers() const { return m_wrappers; }
	const Wrapper& wrappers(size_t i) const { return m_wrappers[i]; }

	const Boosters& boosters() const { return m_boosters; }

	boost::optional<UndoBuffer> move_wrapper(int id, int dir){
		const auto& d = NEIGHBORS[dir];
		auto& w = m_wrappers[id];
		const int length = (w.wheel_remains > 0 ? 2 : 1);
		UndoBuffer ub(id, w, m_boosters);
		bool modified = false;
		for(int iter = 0; iter < length; ++iter){
			const auto p = w.position + d;
			if(!in_range(m_field, p)){ break; }
			if(w.drill_remains == 0 && m_field(p.y, p.x) == CellKind::OBSTACLE){ break; }
			switch(m_field(p.y, p.x)){
			case CellKind::BOOSTER_EX_MANIP:   ++m_boosters.manipulators; break;
			case CellKind::BOOSTER_FAST_WHEEL: ++m_boosters.fast_wheels;  break;
			case CellKind::BOOSTER_DRILL:      ++m_boosters.drills;       break;
			case CellKind::BOOSTER_TELEPORT:   ++m_boosters.teleports;    break;
			case CellKind::BOOSTER_CLONE:      ++m_boosters.clones;       break;
			default: break;
			}
			const auto kind = m_field(p.y, p.x);
			if(kind != CellKind::MYSTERIOUS && kind != CellKind::TELEPORT_BEACON){
				ub.replace(*this, p.y, p.x, CellKind::EMPTY);
			}
			enumerate_reaches(p, w.manipulators, [&](const Vec2& q){ ub.wrap(*this, q.y, q.x); });
			w.position = p;
			modified = true;
		}
		if(!modified){ return {}; }
		if(w.wheel_remains > 0){ --w.wheel_remains; }
		if(w.drill_remains > 0){ --w.drill_remains; }
		return boost::optional<UndoBuffer>(std::move(ub));
	}

	boost::optional<UndoBuffer> rotate_wrapper(int id, int dir){
		auto& w = m_wrappers[id];
		const auto p = w.position;
		UndoBuffer ub(id, w, m_boosters);
		if(dir > 0){
			w.manipulators.rotate_left();
		}else if(dir < 0){
			w.manipulators.rotate_right();
		}
		enumerate_reaches(p, w.manipulators, [&](const Vec2& q){ ub.wrap(*this, q.y, q.x); });
		return boost::optional<UndoBuffer>(std::move(ub));
	}

	boost::optional<UndoBuffer> boost_ex_manip(int id, int dy, int dx){
		if(m_boosters.manipulators == 0){ return {}; }
		auto& w = m_wrappers[id];
		UndoBuffer ub(id, w, m_boosters);
		w.manipulators.insert(Vec2(dx, dy));
		--m_boosters.manipulators;
		return boost::optional<UndoBuffer>(std::move(ub));
	}

	boost::optional<UndoBuffer> boost_fast_wheel(int id){
		if(m_boosters.fast_wheels == 0){ return {}; }
		auto& w = m_wrappers[id];
		UndoBuffer ub(id, w, m_boosters);
		w.wheel_remains = 50;
		--m_boosters.fast_wheels;
		return boost::optional<UndoBuffer>(std::move(ub));
	}

	boost::optional<UndoBuffer> boost_drill(int id){
		if(m_boosters.drills == 0){ return {}; }
		auto& w = m_wrappers[id];
		UndoBuffer ub(id, w, m_boosters);
		w.drill_remains = 30;
		--m_boosters.drills;
		return boost::optional<UndoBuffer>(std::move(ub));
	}

	// TODO teleport

	boost::optional<UndoBuffer> boost_clone(int id){
		if(m_boosters.clones == 0){ return {}; }
		const auto p = m_wrappers[id].position;
		if(m_field(p.y, p.x) != CellKind::MYSTERIOUS){ return {}; }
		UndoBuffer ub(id, m_wrappers[id], m_boosters);
		Wrapper w;
		w.position = p;
		w.manipulators = ManipulatorSet::make_initial_set();
		m_wrappers.push_back(std::move(w));
		--m_boosters.clones;
		return boost::optional<UndoBuffer>(std::move(ub));
	}

	boost::optional<UndoBuffer> evaluate(int id, const Command& cmd){
		if(cmd.kind == CommandKind::MOVE){
			return move_wrapper(id, cmd.args[0]);
		}else if(cmd.kind == CommandKind::ROTATE){
			return rotate_wrapper(id, cmd.args[0]);
		}else if(cmd.kind == CommandKind::EX_MANIP){
			return boost_ex_manip(id, cmd.args[0], cmd.args[1]);
		}else if(cmd.kind == CommandKind::FAST_WHEEL){
			return boost_fast_wheel(id);
		}else if(cmd.kind == CommandKind::DRILL){
			return boost_drill(id);
		}else if(cmd.kind == CommandKind::CLONE){
			return boost_clone(id);
		}else{
			// TODO teleport
			return {};
		}
	}


	std::ostream& dump(std::ostream& os) const {
		for(int i = 0; i < rows(); ++i){
			for(int j = 0; j < cols(); ++j){
				char c = ' ';
				switch(m_field(i, j)){
				case CellKind::OBSTACLE:           c = '#'; break;
				case CellKind::EMPTY:              c = '.'; break;
				case CellKind::MYSTERIOUS:         c = 'X'; break;
				case CellKind::TELEPORT_BEACON:    c = 'T'; break;
				case CellKind::BOOSTER_EX_MANIP:   c = 'B'; break;
				case CellKind::BOOSTER_FAST_WHEEL: c = 'F'; break;
				case CellKind::BOOSTER_DRILL:      c = 'L'; break;
				case CellKind::BOOSTER_TELEPORT:   c = 'B'; break;
				case CellKind::BOOSTER_CLONE:      c = 'C'; break;
				}
				if(m_wrapped(i, j)){
					if(isupper(c)){
						c = tolower(c);
					}else if(c == '.'){
						c = '~';
					}
				}
				os << c;
			}
			os << std::endl;
		}
	}


	static State parse_initial_state(std::istream& is){
		std::vector<std::string> raw_field;
		std::string line;
		while(std::getline(is, line)){ raw_field.push_back(line); }
		const int h = raw_field.size(), w = raw_field[0].size();
		State s;
		s.m_field = Matrix<CellKind>(h, w);
		s.m_wrapped = Matrix<uint8_t>(h, w);
		for(int i = 0; i < h; ++i){
			for(int j = 0; j < w; ++j){
				switch(raw_field[i][j]){
				case '#': s.m_field(i, j) = CellKind::OBSTACLE;           break;
				case '.': s.m_field(i, j) = CellKind::EMPTY;              break;
				case 'X': s.m_field(i, j) = CellKind::MYSTERIOUS;         break;
				case 'B': s.m_field(i, j) = CellKind::BOOSTER_EX_MANIP;   break;
				case 'F': s.m_field(i, j) = CellKind::BOOSTER_FAST_WHEEL; break;
				case 'L': s.m_field(i, j) = CellKind::BOOSTER_DRILL;      break;
				case 'R': s.m_field(i, j) = CellKind::BOOSTER_TELEPORT;   break;
				case 'C': s.m_field(i, j) = CellKind::BOOSTER_CLONE;      break;
				case 'P':
					{
						s.m_field(i, j) = CellKind::EMPTY;
						Wrapper w;
						w.position = Vec2(j, i);
						w.manipulators = ManipulatorSet::make_initial_set();
						w.wheel_remains = 0;
						w.drill_remains = 0;
						s.m_wrappers.push_back(std::move(w));
					}
					break;
				default:
					assert(!"unknown cell type");
				}
			}
		}
		for(const auto& w : s.m_wrappers){
			s.enumerate_reaches(w.position, w.manipulators, [&](const Vec2& q){
				s.m_wrapped(q.y, q.x) = true;
			});
		}
		return s;
	}
};

#endif

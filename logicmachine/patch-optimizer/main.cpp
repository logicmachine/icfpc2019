#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <cassert>
#include <cstdlib>

#include "../common/description_parser.hpp"
#include "../common/command.hpp"
#include "../common/state.hpp"
#include "../common/utility.hpp"


static const int SOURCE_LENGTH = 9;
static const int ITERATION_LIMIT = 5;

inline int manhattan_distance(const Vec2& a, const Vec2& b){
	return abs(a.x - b.x) + abs(a.y - b.y);
}

bool optimize(
	std::vector<Command>& seq,
	int depth,
	size_t wid,
	const Vec2& final_position,
	int final_rotation,
	const std::unordered_map<Vec2, CellKind>& affections,
	int cur_rotation,
	State& state)
{
	const int min_distance =
		  manhattan_distance(state.wrappers(wid).position, final_position)
		+ std::min((cur_rotation - final_rotation) & 3, (final_rotation - cur_rotation) & 3);
	if(depth < min_distance){ return false; }
	if(depth == 0){
		if((final_rotation & 3) != (cur_rotation & 3)){ return false; }
		if(state.wrappers(wid).position != final_position){ return false; }
		for(const auto& a : affections){
			const auto& p = a.first;
			if(!state.wrapped(p.y, p.x)){ return false; }
			if(state.field(p.y, p.x) != a.second){ return false; }
		}
		return true;
	}
	bool accept = false;
	// move
	for(int d = 0; !accept && d < 4; ++d){
		const auto ub = state.move_wrapper(wid, d);
		if(!ub){ continue; }
		seq.emplace_back(CommandKind::MOVE, d);
		accept = optimize(
			seq, depth - 1, wid,
			final_position, final_rotation, affections,
			cur_rotation, state);
		if(!accept){ seq.pop_back(); }
		ub->apply(state);
	}
	// rotate
	for(int d = -1; !accept && d <= 1; d += 2){
		const auto ub = state.rotate_wrapper(wid, d);
		if(!ub){ continue; }
		seq.emplace_back(CommandKind::ROTATE, d);
		accept = optimize(
			seq, depth - 1, wid,
			final_position, final_rotation, affections,
			cur_rotation + d, state);
		if(!accept){ seq.pop_back(); }
		ub->apply(state);
	}
	return accept;
}


int main(int argc, char *argv[]){
	if(argc < 3){
		std::cerr << "Usage: " << argv[0] << " description solution [boosters]" << std::endl;
		return 0;
	}

	const auto description = parse_task_description(read_file(argv[1]));
	std::string raw_solution = read_file(argv[2]);
	while(!raw_solution.empty() && isspace(raw_solution[raw_solution.size() - 1])){
		raw_solution.resize(raw_solution.size() - 1);
	}

	for(int iter = 0; iter < ITERATION_LIMIT; ++iter){
		std::cerr << "------------------------------" << std::endl;
		std::cerr << "  Iteration " << iter << std::endl;
		std::cerr << "------------------------------" << std::endl;
		const auto solution = parse_command_sequence(raw_solution);
		// TODO cloning from wrapper[1-]
		for(size_t i = 1; i < solution.size(); ++i){
			bool has_cloning = false;
			for(const auto& c : solution[i]){
				if(c.kind == CommandKind::CLONE){ has_cloning = true; }
			}
			assert(!has_cloning);
		}

		std::istringstream description_iss(description);
		auto state = State::parse_initial_state(description_iss);

		if(argc >= 4){
			std::ifstream ifs(argv[3]);
			state.initialize_boosters(ifs);
		}

		std::vector<std::vector<Command>> optimized_solution;
		for(size_t wid = 0; wid < solution.size(); ++wid){
			auto sequence = solution[wid];
			for(size_t head = 0; head < sequence.size(); ++head){
				const size_t tail = std::min(head + SOURCE_LENGTH, sequence.size());
				// Forward simulation
				bool has_special = false;
				int rotate = 0;
				std::vector<State::UndoBuffer> ubs;
				ubs.reserve(SOURCE_LENGTH);
				for(size_t i = head; i < tail; ++i){
					const auto k = sequence[i].kind;
					if(k != CommandKind::MOVE && k != CommandKind::ROTATE){ has_special = true; }
					if(k == CommandKind::ROTATE){ rotate += sequence[i].args[0]; }
					ubs.push_back(*state.evaluate(wid, sequence[i]));
				}
				const auto position = state.wrappers(wid).position;
				rotate &= 3;
				// Enumerate affected cells
				std::unordered_map<Vec2, CellKind> affected;
				for(const auto& ub : ubs){
					for(const auto& v : ub.affected_cells()){
						affected.emplace(v, state.field(v.y, v.x));
					}
				}
				// Rollback
				while(!ubs.empty()){
					ubs.back().apply(state);
					ubs.pop_back();
				}
				// ID-DFS
				for(int depth = 0; !has_special && depth + 1 < (tail - head); ++depth){
					std::vector<Command> partial;
					const auto ret = optimize(partial, depth, wid, position, rotate, affected, 0, state);
					if(ret){
						const int remove_length = (tail - head) - depth;
						std::cerr << head << "/" << sequence.size() << " => " << remove_length << std::endl;
						sequence.erase(sequence.begin() + head, sequence.begin() + head + remove_length);
						for(int i = 0; i < depth; ++i){ sequence[head + i] = partial[i]; }
						break;
					}
				}
				// Update state
				state.evaluate(wid, sequence[head]);
			}
			optimized_solution.push_back(sequence);
		}

		std::ostringstream oss;
		for(size_t i = 0; i < optimized_solution.size(); ++i){
			if(i != 0){ oss << "#"; }
			for(const auto& c : optimized_solution[i]){
				oss << c;
			}
		}
		if(oss.str() == raw_solution){ break; }
		raw_solution = oss.str();
	}

	std::cout << raw_solution << std::endl;
	return 0;
}

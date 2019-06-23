#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <cassert>

#include "../common/description_parser.hpp"
#include "../common/command.hpp"
#include "../common/state.hpp"
#include "../common/utility.hpp"


static const int SOURCE_LENGTH = 7;


bool optimize(
	std::vector<Command>& seq,
	int depth,
	const Vec2& final_position,
	int final_rotation,
	const std::unordered_map<Vec2, CellKind>& affections,
	int cur_rotation,
	State& state)
{
	if(depth == 0){
		if((final_rotation & 3) != (cur_rotation & 3)){ return false; }
		if(state.wrappers(0).position != final_position){ return false; }
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
		const auto ub = state.move_wrapper(0, d);
		if(!ub){ continue; }
		seq.emplace_back(CommandKind::MOVE, d);
		accept = optimize(
			seq, depth - 1,
			final_position, final_rotation, affections,
			cur_rotation, state);
		if(!accept){ seq.pop_back(); }
		ub->apply(state);
	}
	// rotate
	for(int d = -1; !accept && d <= 1; d += 2){
		const auto ub = state.rotate_wrapper(0, d);
		if(!ub){ continue; }
		seq.emplace_back(CommandKind::ROTATE, d);
		accept = optimize(
			seq, depth - 1,
			final_position, final_rotation, affections,
			cur_rotation + d, state);
		if(!accept){ seq.pop_back(); }
		ub->apply(state);
	}
	return accept;
}


int main(int argc, char *argv[]){
	if(argc < 3){
		std::cerr << "Usage: " << argv[0] << " description solution" << std::endl;
		return 0;
	}

	const auto description = parse_task_description(read_file(argv[1]));
	const auto solution = parse_command_sequence(read_file(argv[2]));
	assert(solution.size() == 1);  // TODO support cloning

	std::istringstream description_iss(description);
	auto state = State::parse_initial_state(description_iss);

	std::vector<std::vector<Command>> optimized_solution;
	for(auto sequence : solution){
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
				ubs.push_back(*state.evaluate(0, sequence[i]));
			}
			const auto position = state.wrappers(0).position;
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
			for(int depth = 0; !has_special && depth < (tail - head); ++depth){
				std::vector<Command> partial;
				const auto ret = optimize(partial, depth, position, rotate, affected, 0, state);
				if(ret){
					const int remove_length = (tail - head) - depth;
					std::cerr << head << "/" << sequence.size() << " => " << remove_length << std::endl;
					sequence.erase(sequence.begin() + head, sequence.begin() + head + remove_length);
					for(int i = 0; i < depth; ++i){ sequence[head + i] = partial[i]; }
					break;
				}
			}
			// Update state
			state.evaluate(0, sequence[head]);
		}
		state.dump(std::cerr);
		optimized_solution.push_back(sequence);
	}

	for(size_t i = 0; i < optimized_solution.size(); ++i){
		if(i != 0){ std::cout << "#"; }
		for(const auto& c : optimized_solution[i]){
			std::cout << c;
		}
	}
	std::cout << std::endl;
/*
	{
		std::cerr << std::endl;
		std::istringstream description_iss(description);
		auto state = State::parse_initial_state(description_iss);
		for(size_t i = 0; i < optimized_solution.size(); ++i){
			for(const auto& c : optimized_solution[i]){
				state.evaluate(i, c);
				state.dump(std::cerr);
				std::cerr << std::endl;
			}
		}
		state.dump(std::cerr);
	}
*/
	return 0;
}
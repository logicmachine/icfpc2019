#include <iostream>

#include "../common/description_parser.hpp"
#include "../common/command.hpp"
#include "../common/state.hpp"
#include "../common/utility.hpp"


int main(int argc, char *argv[]){
	if(argc < 3){
		std::cerr << "Usage: " << argv[0] << " description solution" << std::endl;
		return 0;
	}
	const auto description = parse_task_description(read_file(argv[1]));
	const auto solution = parse_command_sequence(read_file(argv[2]));

	std::istringstream description_iss(description);
	auto state = State::parse_initial_state(description_iss);

	std::vector<size_t> counters(solution.size());
	size_t timestamp = 0;
	while(true){
		const size_t num_wrappers = state.num_wrappers();
		bool modified = false;
		for(size_t i = 0; i < num_wrappers; ++i){
			if(counters[i] >= solution[i].size()){ continue; }
			const auto& c = solution[i][counters[i]++];
			state.evaluate(i, c);
			modified = true;
		}
		if(!modified){ break; }
		++timestamp;
	}

	state.dump(std::cerr);
	std::cout << timestamp << std::endl;

	return 0;
}

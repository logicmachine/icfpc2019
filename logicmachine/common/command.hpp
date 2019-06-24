#ifndef COMMON_COMMAND_HPP
#define COMMON_COMMAND_HPP

#include <iostream>
#include <string>
#include <array>
#include <cassert>
#include "parser_common.hpp"
#include "utility.hpp"


enum class CommandKind {
	NOP        = 0,
	MOVE       = 1,
	ROTATE     = 2,
	EX_MANIP   = 3,
	FAST_WHEEL = 4,
	DRILL      = 5,
	TELE_RESET = 6,
	TELE_SHIFT = 7,
	CLONE      = 8
};

struct Command {
	CommandKind kind;
	std::array<int, 3> args;

	Command()
		: kind(CommandKind::NOP)
		, args({0, 0, 0})
	{ }

	explicit Command(CommandKind k, int a0 = 0, int a1 = 0, int a2 = 0)
		: kind(k)
		, args({a0, a1, a2})
	{ }
};

inline std::ostream& operator<<(std::ostream& os, const Command& c){
	switch(c.kind){
	case CommandKind::MOVE:       os << "DWAS"[c.args[0]]; break;
	case CommandKind::ROTATE:     os << "E Q"[c.args[0] + 1]; break;
	case CommandKind::EX_MANIP:   os << "B(" << c.args[1] << "," << c.args[0] << ")"; break;
	case CommandKind::FAST_WHEEL: os << "F"; break;
	case CommandKind::DRILL:      os << "L"; break;
	case CommandKind::TELE_RESET: os << "R"; break;
	case CommandKind::TELE_SHIFT: os << "T(" << c.args[1] << "," << c.args[0] << ")"; break;
	case CommandKind::CLONE:      os << "C"; break;
	}
	return os;
}


static std::vector<std::vector<Command>> parse_command_sequence(const std::string& raw){
	const char *s = raw.c_str();
	size_t index = 0;
	std::vector<std::vector<Command>> seqs;
	while(true){
		std::vector<Command> seq;
		while(s[index] != '\0' && s[index] != '#'){
			switch(s[index]){
			case 'W': seq.emplace_back(CommandKind::MOVE, DIR_UP);         ++index; break;
			case 'A': seq.emplace_back(CommandKind::MOVE, DIR_LEFT);       ++index; break;
			case 'S': seq.emplace_back(CommandKind::MOVE, DIR_DOWN);       ++index; break;
			case 'D': seq.emplace_back(CommandKind::MOVE, DIR_RIGHT);      ++index; break;
			case 'Q': seq.emplace_back(CommandKind::ROTATE, ROTATE_LEFT);  ++index; break;
			case 'E': seq.emplace_back(CommandKind::ROTATE, ROTATE_RIGHT); ++index; break;
			case 'F': seq.emplace_back(CommandKind::FAST_WHEEL); ++index; break;
			case 'L': seq.emplace_back(CommandKind::DRILL);      ++index; break;
			case 'R': seq.emplace_back(CommandKind::TELE_RESET); ++index; break;
			case 'C': seq.emplace_back(CommandKind::CLONE);      ++index; break;
			case 'B':
				{
					++index;
					const auto p = detail::parse_point(s, index);
					seq.emplace_back(CommandKind::EX_MANIP, p.y, p.x);
				}
				break;
			case 'T':
				{
					++index;
					const auto p = detail::parse_point(s, index);
					seq.emplace_back(CommandKind::TELE_SHIFT, p.y, p.x);
				}
				break;
			default: ++index;
			}
		}
		seqs.push_back(std::move(seq));
		if(s[index] == '\0'){ break; }
		assert(s[index] == '#'); ++index;
	}
	return seqs;
}

#endif

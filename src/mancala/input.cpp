#include "input.hpp"

#include <cassert>
#include <string>
#include <sstream>
#include <vector>

static std::vector<std::string> split_i(const std::string& s, char d, std::vector<std::string>& v) {
	std::stringstream ss(s);

	std::string cur;
	while(std::getline(ss, cur, d)) {
		v.push_back(cur);
	}

	return v;
}

static std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> toRet;
	split_i(s, delim, toRet);

	return toRet;
}

static uint8_t getNumber(const std::string& s) {
	auto v = std::stoul(s);
	assert(v <= 98);

	return (uint8_t) v;
}

namespace input {

Start* parseStart(std::string& in) {
	assert(in == "START;North" || in == "START;South");
	
	return new Start(in[6] == 'N' ? NORTH : SOUTH);
}

Change* parseChange(std::string& in) {
	std::vector<std::string> tokens = split(in, ';');
	assert(tokens.size() == 4);
	assert(tokens[0] == "CHANGE");

	Board board;
	bool ourTurn;
	uint8_t lastMove;

	
	lastMove = ('1' <= tokens[1][0] && tokens[1][0] < '8') ? tokens[1][0] - '1'
	                                                       : 7;
	std::vector<std::string> stones = split(tokens[2], ',');
	assert(stones.size() == 16);

	//North holes
	for(size_t i = 0; i < 7; i++) {
		board.stonesInHole(NORTH, i) = getNumber(stones[i]);
	}
	
	//North well
	{
		board.stonesInWell(NORTH) = getNumber(stones[7]);
	}

	//South holes
	for(size_t i = 0; i < 7; i++) {
		board.stonesInHole(SOUTH, i) = getNumber(stones[i + 8]);
	}
	
	//South well
	{
		board.stonesInWell(SOUTH) = getNumber(stones[15]);
	}

	board.recalcMoves();

	assert(tokens[3] == "YOU" || tokens[3] == "OPP" || tokens[3] == "END");
	ourTurn = tokens[3][0] == 'Y';

	return new Change(board, ourTurn, lastMove);
}

std::unique_ptr<Message> parseNext(std::istream& in) {
	std::string line;
	std::getline(in, line);

	Message* m = nullptr;
	
	switch(line[0]) {
	case 'C':
		m = parseChange(line); break;
	case 'S':
		m = parseStart(line); break;
	case 'E':
		m = new GameOver; break;
	default:
		m = new NoInput; break;
	}

	return std::unique_ptr<Message>(m);
}

}

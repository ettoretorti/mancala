#include "RandomAgent.hpp"

#include <random>


static thread_local std::default_random_engine gen(std::random_device{}());

uint8_t RandomAgent::makeMove(const Board& b, Side s, bool canSwitch) {
	// 1/8 chance of switching when possible, since there will always be 7 other moves
	if(canSwitch) {
		std::uniform_int_distribution<uint8_t> dist(0, 7);
		if(dist(gen) == 0) {
			return 7;
		}
	}

	size_t nMoves;
	auto* moves = b.validMoves(s, nMoves);
	std::uniform_int_distribution<uint8_t> dist(0, nMoves-1);
	
	return moves[dist(gen)];
}

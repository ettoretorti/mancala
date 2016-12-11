#include "RandomAgent.hpp"

#include <cassert>
#include <random>

static thread_local std::independent_bits_engine<std::default_random_engine, 1, uint8_t> jej(std::random_device{}());
static thread_local std::independent_bits_engine<std::default_random_engine, 2, uint8_t> kek(std::random_device{}());
static thread_local std::independent_bits_engine<std::default_random_engine, 3, uint8_t> lel(std::random_device{}());

static uint8_t rand3() {
	uint8_t s = kek();
	while(s == 3) s = kek();

	return s;
}

static uint8_t randBig(uint8_t n) {
	uint8_t s = lel();
	while(s >= n) s = lel();

	return s;
}

uint8_t __attribute__((hot)) RandomAgent::makeMove(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	// 1/8 chance of switching when possible, since there will always be 7 other moves
	if(movesSoFar == 1) {
		if(lel() == 0) {
			return 7;
		}
	}

	size_t nMoves;
	auto* moves = b.validMoves(s, nMoves);
	assert(nMoves > 0);

	if(nMoves == 1) return moves[0];
	if(nMoves == 2) return moves[jej()];
	if(nMoves == 3) return moves[rand3()];
	if(nMoves == 4) return moves[kek()];
	
	return moves[randBig(nMoves)];
}

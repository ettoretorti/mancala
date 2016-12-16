#pragma once

#include "Agent.hpp"

class MiniMaxAgent : public Agent {
public:
	uint8_t makeMove(const Board& board, Side side, size_t movesSoFar, uint8_t lastMove) override;
	std::pair<uint8_t,double> iterative_deepening(Side toMove, const Board& b, size_t movesSoFar,
												  double time, volatile uint8_t index, volatile double score);
};

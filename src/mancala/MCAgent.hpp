#pragma once

#include <utility>

#include "Agent.hpp"

class MCAgent : public Agent {
public:
	std::pair<uint8_t, float> makeMoveAndScore(const Board& board, Side side, size_t movesSoFar, uint8_t lastMove);
	uint8_t makeMove(const Board& board, Side side, size_t movesSoFar, uint8_t lastMove) override;
};

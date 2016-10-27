#pragma once

#include "Agent.hpp"

class MiniMaxAgent : public Agent {
public:
	uint8_t makeMove(const Board& board, Side side, bool canSwitch);
private:
	uint8_t minimax(uint8_t depth, uint8_t curDepth, Side s, const Board& b, auto* moves);
};

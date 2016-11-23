#pragma once

#include "Agent.hpp"

class MiniMaxAgent : public Agent {
public:
	uint8_t makeMove(const Board& board, Side side, bool canSwitch);
};
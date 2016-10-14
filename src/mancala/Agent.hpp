#pragma once

#include <cstdint>

#include "Board.hpp"

class Agent {
public:
	virtual ~Agent();
	virtual uint8_t makeMove(const Board& board, Side side) = 0;
};

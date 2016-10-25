#pragma once

#include <cstdint>

#include "Board.hpp"

class Agent {
public:
	virtual ~Agent();

	/// The numbers [0, 7) represent holes to move from. Anything >= 7 represents
	/// a pie rule switch, and is only a valid move when canSwitch is true.
	virtual uint8_t makeMove(const Board& board, Side side, bool canSwitch) = 0;
};

#pragma once

#include "Agent.hpp"

#include <functional>
#include <unordered_map>

class MiniMaxAgent : public Agent {
public:
	typedef std::unordered_map<Board, double> MoveCache;
	uint8_t makeMove(const Board& board, Side side, size_t movesSoFar, uint8_t lastMove) override;
	std::pair<uint8_t,double> iterative_deepening(Side toMove, const Board& b, size_t movesSoFar,
												  double time, std::function<void(uint8_t, double)> up);
};

#pragma once

#include <utility>

#include "Agent.hpp"

class MCAgentImplicitHeuristics : public Agent {
public:
	MCAgentImplicitHeuristics(uint16_t ucbDepth, uint16_t ucbBaseGames, uint32_t iterations);
	MCAgentImplicitHeuristics() : MCAgentImplicitHeuristics(7, 3, 7500) {}

	std::pair<uint8_t, float> makeMoveAndScore(const Board& board, Side side, size_t movesSoFar, uint8_t lastMove);
	uint8_t makeMove(const Board& board, Side side, size_t movesSoFar, uint8_t lastMove) override;

	uint16_t& depth();
	uint16_t& baseGames();
	uint32_t& iterations();

	float& timePerMove();
	bool& useIterations();

private:
	uint16_t depth_;
	uint16_t baseGames_;
	uint32_t iterations_;

	float timePerMove_;
	bool useIterations_;
};

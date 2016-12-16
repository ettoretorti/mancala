#pragma once

#include <utility>

#include "Agent.hpp"

class MCAgent : public Agent {
public:
	MCAgent(uint32_t bufSize, uint16_t ucbBaseGames, uint32_t iterations);
	MCAgent() : MCAgent(500000, 1, 100000) {}

	std::pair<uint8_t, float> makeMoveAndScore(const Board& board, Side side, size_t movesSoFar, uint8_t lastMove);
	uint8_t makeMove(const Board& board, Side side, size_t movesSoFar, uint8_t lastMove) override;

	uint32_t& bufferSize();
	uint16_t& baseGames();
	uint32_t& iterations();

	float& timePerMove();
	bool& useIterations();

private:
	uint32_t bufSize_;
	uint16_t baseGames_;
	uint32_t iterations_;

	float timePerMove_;
	bool useIterations_;
};

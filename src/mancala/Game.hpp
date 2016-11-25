#pragma once

#include <memory>

#include "Board.hpp"
#include "Agent.hpp"


class Game {
public:
	Game(std::unique_ptr<Agent> p1, std::unique_ptr<Agent> p2);
	Game(Agent* p1, Agent* p2);

	Board& board();
	const Board& board() const;

	Side& toMove();
	Side toMove() const;

	void reset();

	bool isOver() const;
	bool sidesSwapped() const;
	bool& sidesSwapped();

	size_t movesPlayed() const;

	void stepTurn();
	void playAll();

	/// p1 score - p2 score
	/// only to be called when isOver() returns true
	int scoreDifference();

private:
	Board board_;
	std::unique_ptr<Agent> p1_;
	std::unique_ptr<Agent> p2_;
	Side toMove_;
	size_t movesPlayed_;
	uint8_t lastMove_;
	bool sidesSwapped_;
};

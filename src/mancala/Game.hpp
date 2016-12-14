#pragma once

#include <memory>

#include "Board.hpp"
#include "Agent.hpp"


class Game {
public:
	Game(std::unique_ptr<Agent> p1, std::unique_ptr<Agent> p2);
	Game(Agent* p1, Agent* p2);

	inline Board& board() { return board_; }
	inline const Board& board() const { return board_; }

	Side& toMove();
	Side toMove() const;

	void reset();

	// copies everything but the players
	void copyState(const Game& other);

	inline bool isOver() const;
	bool sidesSwapped() const;
	bool& sidesSwapped();

	size_t movesPlayed() const;
	size_t& movesPlayed();

	uint8_t lastMove() const;
	uint8_t& lastMove();

	double p1Time() const;
	double& p1Time();

	double p2Time() const;
	double& p2Time();

	bool& takeTimings();

	void stepTurn();
	void playAll();

	/// p1 score - p2 score
	/// only to be called when isOver() returns true
	int scoreDifference();

private:
	Board board_;
	std::unique_ptr<Agent> p1_;
	std::unique_ptr<Agent> p2_;
	double p1Time_;
	double p2Time_;
	Side toMove_;
	size_t movesPlayed_;
	uint8_t lastMove_;
	bool sidesSwapped_;
	bool takeTimings_;
};

inline bool Game::isOver() const {
	size_t nValidMoves;
	board_.validMoves(toMove_, nValidMoves);

	return nValidMoves == 0;
}

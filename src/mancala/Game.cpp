#include <cassert>

#include "Game.hpp"

Game::Game(std::unique_ptr<Agent> p1, std::unique_ptr<Agent> p2)
	: board_(), p1_(std::move(p1)), p2_(std::move(p2)), toMove_(SOUTH),
	  movesPlayed_(0), sidesSwapped_(false)
{}


Game::Game(Agent* p1, Agent* p2)
	: board_(), p1_(p1), p2_(p2), toMove_(SOUTH), movesPlayed_(0),
	  sidesSwapped_(false)
{}

Board& Game::board() {
	return board_;
}

const Board& Game::board() const {
	return board_;
}

bool Game::sidesSwapped() const {
	return sidesSwapped_;
}

bool& Game::sidesSwapped() {
	return sidesSwapped_;
}

Side& Game::toMove() {
	return toMove_;
}

Side Game::toMove() const {
	return toMove_;
}

void Game::reset() {
	board_.reset();
	toMove_ = SOUTH;
	movesPlayed_ = 0;
	sidesSwapped_ = false;
}

bool Game::isOver() const {
	size_t nValidMoves;
	board_.validMoves(toMove_, nValidMoves);

	return nValidMoves == 0;
}

void Game::stepTurn() {
	assert(!isOver());

	bool canSwitch = movesPlayed_ == 1;

	uint8_t move = toMove_ == SOUTH && !sidesSwapped_ ? p1_->makeMove(board_, toMove_, canSwitch) :
	               toMove_ == NORTH && !sidesSwapped_ ? p2_->makeMove(board_, toMove_, canSwitch) :
	               toMove_ == SOUTH &&  sidesSwapped_ ? p2_->makeMove(board_, toMove_, canSwitch) :
	                                                    p1_->makeMove(board_, toMove_, canSwitch);

	assert(canSwitch || move < 7);

	if(canSwitch && move >= 7) {
		sidesSwapped_ = true;
	} else {
		#ifndef NDEBUG
			size_t nMoves;
			const uint8_t* moves = board_.validMoves(toMove_, nMoves);
			bool moveValid = false;
			for(size_t i = 0; i < nMoves; i++) {
				moveValid |= moves[i] == move;
			}
			assert(moveValid);
		#endif
		if(!board_.makeMove(toMove_, move) || movesPlayed_ == 0)
			toMove_ = (Side)((int)toMove_^1);
	}

	movesPlayed_++;
}

void Game::playAll() {
	while(!isOver()) stepTurn();
}

int Game::scoreDifference() {
	assert(isOver());

	//clean up the board by giving the winning player all the remaining stones
	Side winner = (Side)((int)toMove_^1);

	for(size_t i = 0; i < 7; i++) {
		board_.stonesInWell(winner) += board_.stonesInHole(winner, i);
		board_.stonesInHole(winner, i) = 0;
	}

	return board_.stonesInWell(!sidesSwapped_ ? SOUTH : NORTH)
	       - board_.stonesInWell(!sidesSwapped_ ? NORTH : SOUTH);
}

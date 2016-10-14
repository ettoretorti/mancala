#include <cassert>

#include "Game.hpp"

Game::Game(std::unique_ptr<Agent> p1, std::unique_ptr<Agent> p2)
	: board_(), p1_(std::move(p1)), p2_(std::move(p2)), toMove_(SOUTH)
{}


Game::Game(Agent* p1, Agent* p2)
	: board_(), p1_(p1), p2_(p2), toMove_(SOUTH)
{}

Board& Game::board() {
	return board_;
}

const Board& Game::board() const {
	return board_;
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
}

bool Game::isOver() const {
	size_t nValidMoves;
	board_.validMoves(toMove_, nValidMoves);

	return nValidMoves == 0;
}

void Game::stepTurn() {
	assert(!isOver());

	uint8_t move = toMove_ == SOUTH ? p1_->makeMove(board_, SOUTH)
	                                : p2_->makeMove(board_, NORTH);

#ifndef NDEBUG
	size_t nMoves;
	const uint8_t* moves = board_.validMoves(toMove_, nMoves);
	bool moveValid = false;
	for(size_t i = 0; i < nMoves; i++) {
		moveValid |= moves[i] == move;
	}

	assert(moveValid);
#endif

	board_.makeMove(toMove_, move);
	toMove_ = (Side)((int)toMove_^1);
}

void Game::playAll() {
	while(!isOver()) stepTurn();
}

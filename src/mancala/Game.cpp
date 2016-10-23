#include <cassert>

#include "Game.hpp"

Game::Game(std::unique_ptr<Agent> p1, std::unique_ptr<Agent> p2)
	: board_(), p1_(std::move(p1)), p2_(std::move(p2)), toMove_(SOUTH),
	  p1Side_(SOUTH), p2Side_(NORTH)
{}


Game::Game(Agent* p1, Agent* p2)
	: board_(), p1_(p1), p2_(p2), toMove_(SOUTH), p1Side_(SOUTH), p2Side_(NORTH)
{}

Board& Game::board() {
	return board_;
}

const Board& Game::board() const {
	return board_;
}

Side& Game::p1Side() {
	return p1Side_;
}

Side& Game::p2Side() {
	return p2Side_;
}

Side Game::p1Side() const {
	return p1Side_;
}

Side Game::p2Side() const {
	return p2Side_;
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
	p1Side_ = SOUTH;
	p2Side_ = NORTH;
}

bool Game::isOver() const {
	size_t nValidMoves;
	board_.validMoves(toMove_, nValidMoves);

	return nValidMoves == 0;
}

void Game::stepTurn() {
	assert(!isOver());

	uint8_t move = toMove_ == p1Side_ ? p1_->makeMove(board_, p1Side_)
	                                : p2_->makeMove(board_, p2Side_);

	if(move == SWITCH){
		p1Side_ = p1Side_ == SOUTH? NORTH : SOUTH;
		p2Side_ = p2Side_ == SOUTH? NORTH : SOUTH;	
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
		board_.makeMove(toMove_, move);
		toMove_ = (Side)((int)toMove_^1);
	}
}

void Game::playAll() {
	while(!isOver()) stepTurn();
}

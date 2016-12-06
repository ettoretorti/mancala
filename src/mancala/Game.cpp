#include "Game.hpp"

#include <cassert>
#include <chrono>

Game::Game(std::unique_ptr<Agent> p1, std::unique_ptr<Agent> p2)
	: board_(), p1_(std::move(p1)), p2_(std::move(p2)), p1Time_(0.0), p2Time_(0.0),
	  toMove_(SOUTH), movesPlayed_(0), sidesSwapped_(false), takeTimings_(false)
{}


Game::Game(Agent* p1, Agent* p2)
	: board_(), p1_(p1), p2_(p2), p1Time_(0.0), p2Time_(0.0),
	  toMove_(SOUTH), movesPlayed_(0), sidesSwapped_(false), takeTimings_(false)
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

void Game::copyState(const Game& other) {
	board_ = other.board_;
	toMove_ = other.toMove_;
	movesPlayed_ = other.movesPlayed_;
	sidesSwapped_ = other.sidesSwapped_;
}

bool Game::isOver() const {
	size_t nValidMoves;
	board_.validMoves(toMove_, nValidMoves);

	return nValidMoves == 0;
}

size_t Game::movesPlayed() const {
	return movesPlayed_;
}

size_t& Game::movesPlayed() {
	return movesPlayed_;
}

uint8_t Game::lastMove() const {
	return lastMove_;
}

uint8_t& Game::lastMove() {
	return lastMove_;
}

double Game::p1Time() const {
	return p1Time_;
}

double& Game::p1Time() {
	return p1Time_;
}

double Game::p2Time() const {
	return p2Time_;
}

double& Game::p2Time() {
	return p2Time_;
}

bool& Game::takeTimings() {
	return takeTimings_;
}

void Game::stepTurn() {
	using namespace std::chrono;

	assert(!isOver());

	bool canSwitch = movesPlayed_ == 1;
	
	int playerToMove = toMove_ == SOUTH && !sidesSwapped_ ? 1 :
	                   toMove_ == NORTH && !sidesSwapped_ ? 2 :
	                   toMove_ == SOUTH &&  sidesSwapped_ ? 2 :
	                                                       1;  
	
	uint8_t move;

	if(takeTimings_) {
		auto before = high_resolution_clock::now();

		move = playerToMove == 1 ? p1_->makeMove(board_, toMove_, movesPlayed_, lastMove_) :
		                           p2_->makeMove(board_, toMove_, movesPlayed_, lastMove_);
		
		auto after = high_resolution_clock::now();
		double timeTaken = duration_cast<duration<double>>(after - before).count();

		if(playerToMove == 1) p1Time_ += timeTaken;
		else                  p2Time_ += timeTaken;
	} else {
		move = playerToMove == 1 ? p1_->makeMove(board_, toMove_, movesPlayed_, lastMove_) :
		                           p2_->makeMove(board_, toMove_, movesPlayed_, lastMove_);
	}

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

	lastMove_ = move;
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

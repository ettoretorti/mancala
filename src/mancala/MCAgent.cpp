#include "MCAgent.hpp"

#include "RandomAgent.hpp"
#include "Game.hpp"

#include <random>
#include <memory>
#include <cmath>
#include <tuple>
#include <chrono>

struct UCB {
	Board board;
	uint32_t plays = 0;
	uint32_t wins[2] = { 0 };
	Side whosTurn;
};

size_t ipow(size_t base, size_t exp, size_t res = 1) {
	return exp == 0 ? res : ipow(base, exp-1, res * base);
}

static inline Side opposite(Side s) {
	return (Side)(((int)s) ^ 1);
}

static inline size_t childIdx(size_t idx, size_t move) {
	return 7 * idx + move + 1;
}

MCAgent::MCAgent(uint16_t ucbDepth, uint16_t ucbBaseGames, uint32_t iterations)
	: depth_(ucbDepth), baseGames_(ucbBaseGames), iterations_(iterations), timePerMove_(1.0), useIterations_(true)
{}

uint16_t& MCAgent::depth() {
	return depth_;
}

uint16_t& MCAgent::baseGames() {
	return baseGames_;
}

uint32_t& MCAgent::iterations() {
	return iterations_;
}

float& MCAgent::timePerMove() {
	return timePerMove_;
}

bool& MCAgent::useIterations() {
	return useIterations_;
}

static std::tuple<uint32_t, uint32_t> montecarlo(Side ourSide, UCB* ucbs, size_t depth, size_t idx, size_t baseGames);

uint8_t MCAgent::makeMove(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	return makeMoveAndScore(b, s, movesSoFar, lastMove).first;
}

std::pair<uint8_t, float> MCAgent::makeMoveAndScore(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	using namespace std::chrono;

	// look at notes/WHEN_TO_PIE for details
	if(movesSoFar == 0) return std::make_pair(6, 0.5);
	if(movesSoFar == 1 && (lastMove == 0 || lastMove == 4 || lastMove == 5 || lastMove == 6)) return std::make_pair(7, 0.5);

	auto ucbs = std::unique_ptr<UCB[]>(new UCB[ipow(7, depth_)]);
	
	size_t nMoves;
	const uint8_t* moves = b.validMoves(s, nMoves);
	
	ucbs[0].board = b;
	ucbs[0].whosTurn = s;

	if(useIterations_) {
		for(size_t i = 0; i < iterations_; i++) {
			montecarlo(s, ucbs.get(), depth_ - 1, 0, baseGames_);
		}
	} else {
		auto deadline = high_resolution_clock::now() + duration<double>(timePerMove_);

		auto t1 = high_resolution_clock::now();
		montecarlo(s, ucbs.get(), depth_ - 1, 0, baseGames_);
		auto t2 = high_resolution_clock::now();

		size_t itsCompleted = ucbs[0].plays / 2 / baseGames_;

		while(t2 < deadline) {
			double itsPerSec = itsCompleted / duration_cast<duration<double>>(t2 - t1).count();
			itsCompleted = std::max(size_t(1), size_t(itsPerSec * duration_cast<duration<double>>(deadline - t2).count()));

			t1 = high_resolution_clock::now();
			for(size_t i = 0; i < itsCompleted; i++) {
				montecarlo(s, ucbs.get(), depth_ - 1, 0, baseGames_);
			}
			t2 = high_resolution_clock::now();
		}

	}

	size_t bestMove = 0;
	double bestScore = -1.0/0.0;

	for(size_t i = 0; i < nMoves; i++) {
		double score = ucbs[moves[i] + 1].wins[(int)s] / (double) ucbs[moves[i] + 1].plays;

		if(score > bestScore) {
			bestScore = score;
			bestMove = moves[i];
		}
	}

	return std::make_pair(bestMove, bestScore);
}

// This is hacky as fuck, but will do for now
static thread_local Game g(new RandomAgent, new RandomAgent);

// South score, north score
static std::tuple<uint32_t, uint32_t> montecarlo(Side ourSide, UCB* ucbs, size_t depth, size_t idx, size_t baseGames) {
	UCB& cur = ucbs[idx];
	Side toMove = cur.whosTurn;
	Side opp = opposite(toMove);


	size_t nMoves;
	const auto* moves = cur.board.validMoves(toMove, nMoves);

	// The game is over
	if(nMoves == 0) {
		//cleanup for the opponent
		for(uint8_t i = 0; i < 7; i++) {
			cur.board.stonesInWell(opp) += cur.board.stonesInHole(toMove, i);
			cur.board.stonesInHole(toMove, i) = 0;
		}

		//determine who won and update accordingly
		uint8_t southScore = cur.board.stonesInWell(SOUTH);
		uint8_t northScore = cur.board.stonesInWell(NORTH);

		cur.plays+= 2 * baseGames;

		if(southScore > northScore) {
			cur.wins[0] += 2 * baseGames;
			return std::make_tuple((uint32_t)baseGames, 0u);
		}

		if(southScore < northScore) {
			cur.wins[1] +=  2 * baseGames;
			return std::make_tuple(0u, (uint32_t)baseGames);
		}

		cur.wins[0] += baseGames;
		cur.wins[1] += baseGames;

		return std::make_tuple((uint32_t)baseGames, (uint32_t)baseGames);
	}

	if(depth == 0) {
		uint32_t wins[2] = { 0 };

		for(size_t i = 0; i < baseGames; i++) {
			g.board() = cur.board;
			g.movesPlayed() = 3; // to avoid switching
			g.toMove() = toMove;

			g.playAll();
			
			int diff = g.scoreDifference();
			if(diff > 0) wins[0] += 2;
			if(diff < 0) wins[1] += 2;
			if(diff == 0) {
				wins[0]++;
				wins[1]++;
			}
		}

		cur.plays += 2 * baseGames;
		cur.wins[0] += wins[0];
		cur.wins[1] += wins[1];

		return std::make_tuple(wins[0], wins[1]);
	}
	
	// Populate children if necessary
	if(cur.plays == 0) {
		uint32_t wins[2] = { 0 };
		for(size_t i = 0; i < nMoves; i++) {
			uint8_t move = moves[i];
			UCB& child = ucbs[childIdx(idx, move)];

			child.board = cur.board;
			child.board.makeMove(ourSide, move);

			child.whosTurn = opp;

			auto res = montecarlo(ourSide, ucbs, depth - 1 , childIdx(idx, move), baseGames);

			cur.plays += child.plays;
			wins[0] += std::get<0>(res);
			wins[1] += std::get<1>(res);

		}
		cur.wins[0] += wins[0];
		cur.wins[1] += wins[1];

		return std::make_tuple(wins[0], wins[1]);
	}
	
	// go by max bound otherwise
	double logTotal = 1.0 * log(cur.plays);
	double  max = -1.0/0.0;
	size_t move = moves[0];

	for(size_t i = 0; i < nMoves; i++) {
		UCB& child = ucbs[idx * 7 + moves[i]];
		
		double bound = child.wins[(int)toMove] / (float) child.plays;
		bound += sqrt(logTotal / child.plays);

		if(bound > max) {
			max = bound;
			move = moves[i];
		}
	}

	size_t childI = childIdx(idx, move);
	uint32_t curPlays = ucbs[childI].plays;

	auto res = montecarlo(ourSide, ucbs, depth - 1, childI, baseGames);
	
	cur.wins[0] += std::get<0>(res);
	cur.wins[1] += std::get<1>(res);
	cur.plays += ucbs[childI].plays - curPlays;

	return res;
}

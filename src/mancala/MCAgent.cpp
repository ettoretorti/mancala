#include "MCAgent.hpp"

#include "RandomAgent.hpp"
#include "Game.hpp"

#include <random>
#include <memory>
#include <cmath>

struct UCB {
	Board board;
	uint32_t plays = 0;
	uint32_t wins = 0;
	Side whosTurn;
};

constexpr size_t ipow(size_t base, size_t exp, size_t res = 1) {
	return exp == 0 ? res : ipow(base, exp-1, res * base);
}

constexpr size_t UCB_DEPTH = 5;
constexpr size_t UCB_POOL_SIZE = ipow(7, UCB_DEPTH);
constexpr size_t UCB_BASE_GAMES = 20;

static inline Side opposite(Side s) {
	return (Side)(((int)s) ^ 1);
}

static inline size_t childIdx(size_t idx, size_t move) {
	return 7 * idx + move + 1;
}

static void montecarlo(Side ourSide, UCB* ucbs, size_t depth, size_t idx);

uint8_t MCAgent::makeMove(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	//always swap for now
	if(movesSoFar == 1) return 7;

	auto ucbs = std::unique_ptr<UCB[]>(new UCB[UCB_POOL_SIZE]);
	
	size_t nMoves;
	const uint8_t* moves = b.validMoves(s, nMoves);
	
	ucbs[0].board = b;
	ucbs[0].whosTurn = s;

	for(size_t i = 0; i < 7500; i++) {
		montecarlo(s, ucbs.get(), 0, 0);
	}

	size_t bestMove = 0;
	double bestScore = -1.0/0.0;

	for(size_t i = 0; i < nMoves; i++) {
		double score = ucbs[moves[i] + 1].wins / (float) ucbs[moves[i] + 1].plays;

		if(score > bestScore) {
			bestScore = score;
			bestMove = moves[i];
		}
	}

	return bestMove;
}

// This is hacky as fuck, but will do for now
static Game g(new RandomAgent, new RandomAgent);

static void montecarlo(Side ourSide, UCB* ucbs, size_t depth, size_t idx) {
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
		uint8_t ourScore = cur.board.stonesInWell(ourSide);
		uint8_t oppScore = cur.board.stonesInWell(opposite(ourSide));

		cur.plays+= UCB_BASE_GAMES;
		if(ourScore > oppScore) cur.wins += UCB_BASE_GAMES;

		return;
	}

	if(depth == UCB_DEPTH - 1) {
		auto isWin = ourSide == SOUTH ? [](int x) { return x > 0;} : [](int x) { return x < 0; };

		for(size_t i = 0; i < UCB_BASE_GAMES; i++) {
			g.board() = cur.board;
			g.movesPlayed() = 3; // to avoid switching
			g.toMove() = toMove;

			g.playAll();

			if(isWin(g.scoreDifference())) cur.wins++;
		}

		cur.plays += UCB_BASE_GAMES;

		return;
	}
	
	// Populate children if necessary
	if(cur.plays == 0) {
		for(size_t i = 0; i < nMoves; i++) {
			uint8_t move = moves[i];
			UCB& child = ucbs[childIdx(idx, move)];

			child.board = cur.board;
			child.board.makeMove(ourSide, move);

			child.whosTurn = opp;

			montecarlo(ourSide, ucbs, depth + 1 , childIdx(idx, move));

			cur.plays += child.plays;
			cur.wins  += child.wins;
		}

		return;
	}
	
	// go by max bound otherwise
	double logTotal = 2 * log(cur.plays);
	double  max = -1.0/0.0;
	size_t move = moves[0];

	for(size_t i = 0; i < nMoves; i++) {
		UCB& child = ucbs[idx * 7 + moves[i]];
		
		double bound = child.wins / (float) child.plays;
		bound += sqrt(logTotal / child.plays);

		if(bound > max) {
			max = bound;
			move = moves[i];
		}
	}

	size_t childI = childIdx(idx, move);
	uint32_t curWins = ucbs[childI].wins;
	uint32_t curPlays = ucbs[childI].plays;

	montecarlo(ourSide, ucbs, depth + 1, childI);
	
	cur.wins += ucbs[childI].wins - curWins;
	cur.plays += ucbs[childI].plays - curPlays;
}

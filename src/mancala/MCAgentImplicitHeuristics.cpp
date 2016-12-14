#include "MCAgentImplicitHeuristics.hpp"

#include "RandomAgent.hpp"
#include "Game.hpp"

#include <cassert>
#include <random>
#include <limits>
#include <memory>
#include <cmath>
#include <tuple>
#include <chrono>

struct UCB {
	Board board;
	uint64_t plays = 0;
	uint64_t wins[2] = { 0 };
	Side whosTurn;
	double heuristic = 0.0;
};

static size_t ipow(size_t base, size_t exp, size_t res = 1) {
	return exp == 0 ? res : ipow(base, exp-1, res * base);
}

static inline Side opposite(Side s) {
	return (Side)(((int)s) ^ 1);
}

static inline size_t childIdx(size_t idx, size_t move) {
	return 7 * idx + move + 1;
}

MCAgentImplicitHeuristics::MCAgentImplicitHeuristics(uint16_t ucbDepth, uint16_t ucbBaseGames, uint32_t iterations)
	: depth_(ucbDepth), baseGames_(ucbBaseGames), iterations_(iterations), timePerMove_(1.0), useIterations_(true)
{}

uint16_t& MCAgentImplicitHeuristics::depth() {
	return depth_;
}

uint16_t& MCAgentImplicitHeuristics::baseGames() {
	return baseGames_;
}

uint32_t& MCAgentImplicitHeuristics::iterations() {
	return iterations_;
}

float& MCAgentImplicitHeuristics::timePerMove() {
	return timePerMove_;
}

bool& MCAgentImplicitHeuristics::useIterations() {
	return useIterations_;
}

static std::tuple<uint32_t, uint32_t> montecarlo(UCB* ucbs, size_t depth, size_t idx, size_t baseGames);

uint8_t MCAgentImplicitHeuristics::makeMove(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	return makeMoveAndScore(b, s, movesSoFar, lastMove).first;
}

std::pair<uint8_t, float> MCAgentImplicitHeuristics::makeMoveAndScore(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	using namespace std::chrono;

	// look at notes/WHEN_TO_PIE for details
	if(movesSoFar == 0) return std::make_pair(0, 492.5/507.5);
	if(movesSoFar == 1 && (lastMove == 1 || lastMove == 3 || lastMove == 4 || lastMove == 5 || lastMove == 6)) return std::make_pair(7, 0.5);

	// instant win/loss
	if(b.stonesInWell(s) > 49) {
		return std::make_pair(RandomAgent().makeMove(b, s, movesSoFar, lastMove), 1.0);
	}
	if(b.stonesInWell(Side(int(s)^1)) > 49) {
		return std::make_pair(RandomAgent().makeMove(b, s, movesSoFar, lastMove), 0.0);
	}

	auto ucbs = std::unique_ptr<UCB[]>(new UCB[ipow(7, depth_)]);
	
	size_t nMoves;
	const uint8_t* moves = b.validMoves(s, nMoves);
	assert(nMoves > 0);
 	
	ucbs[0].board = b;
	ucbs[0].whosTurn = s;

	if(useIterations_) {
		for(size_t i = 0; i < iterations_; i++) {
			montecarlo(ucbs.get(), depth_ - 1, 0, baseGames_);
		}
	} else {
		auto deadline = high_resolution_clock::now() + duration<double>(timePerMove_);

		auto t1 = high_resolution_clock::now();
		montecarlo(ucbs.get(), depth_ - 1, 0, baseGames_);
		auto t2 = high_resolution_clock::now();

		size_t itsCompleted = ucbs[0].plays / 2 / baseGames_;

		while(t2 < deadline) {
			double itsPerSec = itsCompleted / duration_cast<duration<double>>(t2 - t1).count();
			itsCompleted = std::max(size_t(1), size_t(itsPerSec * duration_cast<duration<double>>(deadline - t2).count()));

			t1 = high_resolution_clock::now();
			for(size_t i = 0; i < itsCompleted; i++) {
				montecarlo(ucbs.get(), depth_ - 1, 0, baseGames_);
			}
			t2 = high_resolution_clock::now();
		}
	}

	size_t bestMove = moves[0];
	double bestScore = -std::numeric_limits<double>::infinity();
	size_t mostPlays = 0;

	for(size_t i = 0; i < nMoves; i++) {
		double score = (ucbs[moves[i] + 1].wins[(int)s] / (double) ucbs[moves[i] + 1].plays) + ucbs[moves[i]+1].heuristic;
		size_t plays = ucbs[moves[i] + 1].plays;

		if(plays > mostPlays) {
			mostPlays = plays;
			bestScore = score;
			bestMove = moves[i];
		}
	}

	return std::make_pair(bestMove, bestScore);
}

// This is hacky as fuck, but will do for now
static thread_local Game g(new RandomAgent, new RandomAgent);

// South score, north score
static std::tuple<uint32_t, uint32_t> montecarlo(UCB* ucbs, size_t depth, size_t idx, size_t baseGames) {
	UCB cur = ucbs[idx];
	Side toMove = cur.whosTurn;
	Side opp = opposite(toMove);

	cur.heuristic += cur.board.stonesInWell(SOUTH) - cur.board.stonesInWell(NORTH);
	// Guaranteed win/loss ;)
	if(cur.board.stonesInWell(SOUTH) > 49) {
		cur.plays += 2 * baseGames;
		cur.wins[0] += 2 * baseGames;
		return std::make_tuple(uint32_t(2 * baseGames), uint32_t(0));
	}

	if(cur.board.stonesInWell(NORTH) > 49) {
		cur.plays += 2 * baseGames;
		cur.wins[1] += 2 * baseGames;
		return std::make_tuple(uint32_t(0), uint32_t(2 * baseGames));
	}

	size_t nMoves;
	const auto* moves = cur.board.validMoves(toMove, nMoves);

	// The game is over
	if(nMoves == 0) {
		//determine who won and update accordingly
		uint8_t scores[2] = { cur.board.stonesInWell(SOUTH), cur.board.stonesInWell(NORTH) };
		scores[opp] += 98 - scores[0] - scores[1];

		cur.plays+= 2 * baseGames;
		cur.heuristic += (scores[0] - scores[1]);

		if(scores[0] > scores[1]) {
			cur.wins[0] += 2 * baseGames;
			return std::make_tuple((uint32_t)2 * baseGames, 0u);
		}

		if(scores[0] < scores[1]) {
			cur.wins[1] +=  2 * baseGames;
			return std::make_tuple(0u, (uint32_t)2 * baseGames);
		}

		cur.wins[0] += baseGames;
		cur.wins[1] += baseGames;
		return std::make_tuple((uint32_t)baseGames, (uint32_t)baseGames);
	}

	if(depth == 0) {
		uint32_t wins[2] = { 0 };
		double heuristic = 0.0;
		for(size_t i = 0; i < baseGames; i++) {
			g.board() = cur.board;
			g.movesPlayed() = 3; // to avoid switching
			g.toMove() = toMove;

			while(g.board().stonesInWell(SOUTH) <= 49 && g.board().stonesInWell(NORTH) <= 49 && !g.isOver()) g.stepTurn();
			
			heuristic += (g.board().stonesInWell(SOUTH) - g.board().stonesInWell(NORTH));
			if(g.board().stonesInWell(SOUTH) > 49) {
				wins[0] += 2;
			} else if(g.board().stonesInWell(NORTH) > 49) {
				wins[1] += 2;
			} else {
				uint8_t scores[2] = { cur.board.stonesInWell(SOUTH), cur.board.stonesInWell(NORTH) };
				scores[int(g.toMove())^1] += 98 - scores[0] - scores[1];

				int diff = int(scores[0]) - scores[1];
				if(diff > 0) wins[0] += 2;
				if(diff < 0) wins[1] += 2;
				if(diff == 0) {
					wins[0]++;
					wins[1]++;
				}
			}
		}

		cur.plays += 2 * baseGames;
		cur.wins[0] += wins[0];
		cur.wins[1] += wins[1];
		cur.heuristic += heuristic;
		return std::make_tuple(wins[0], wins[1]);
	}
	
	// Populate children if necessary
	if(cur.plays == 0) {
		uint32_t wins[2] = { 0 };
		double heuristic = 0.0;
		for(size_t i = 0; i < nMoves; i++) {
			uint8_t move = moves[i];
			UCB child = ucbs[childIdx(idx, move)];

			child.board = cur.board;
			bool ga = child.board.makeMove(toMove, move);

			child.whosTurn = ga ? toMove : opp;

			auto res = montecarlo(ucbs, depth - 1, childIdx(idx, move), baseGames);

			cur.plays += child.plays;
			wins[0] += std::get<0>(res);
			wins[1] += std::get<1>(res);
			heuristic += child.heuristic;
		}
		cur.wins[0] += wins[0];
		cur.wins[1] += wins[1];
		cur.heuristic = heuristic;
		return std::make_tuple(wins[0], wins[1]);
	}
	
	// go by max bound otherwise
	double logTotal = 2.0 * log(cur.plays);
	double max = -std::numeric_limits<double>::infinity();
	size_t move = moves[0];

	for(size_t i = 0; i < nMoves; i++) {
		UCB child = ucbs[childIdx(idx, moves[i])];
		
		double bound = child.wins[(int)toMove] / (float) child.plays;
		bound += sqrt(logTotal / child.plays);
		bound += child.heuristic;

		if(bound > max) {
			max = bound;
			move = moves[i];
		}
	}

	size_t childI = childIdx(idx, move);
	uint64_t curPlays = ucbs[childI].plays;

	auto res = montecarlo(ucbs, depth - 1, childI, baseGames);
	
	cur.wins[0] += std::get<0>(res);
	cur.wins[1] += std::get<1>(res);
	cur.plays += ucbs[childI].plays - curPlays;
	cur.heuristic += ucbs[childI].heuristic;
	return res;
}

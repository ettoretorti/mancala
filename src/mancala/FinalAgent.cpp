#include "FinalAgent.hpp"
#include "RandomAgent.hpp"
#include "MiniMaxAgent.hpp"
#include "Game.hpp"

#include <cassert>
#include <random>
#include <limits>
#include <memory>
#include <cmath>
#include <tuple>
#include <chrono>
#include <unordered_map>
#include <iostream>

static MiniMaxAgent minimaxAgent;

struct UCB {
	Board board;
	uint64_t plays = 0;
	uint64_t wins[2] = { 0 };
	Side whosTurn;
	uint32_t childIdxs[7];
};

static inline size_t childIdx(size_t idx, size_t move) {
	return 7 * idx + move + 1;
}

static size_t neededSize(int depth) {
	size_t initial = 0;

	for(int i = 0; i < depth - 1; i++) {
		initial = childIdx(initial, 6);
	}

	return initial + 1;
}

size_t ipow(size_t base, size_t exp, size_t res = 1) {
	return exp == 0 ? res : ipow(base, exp-1, res * base);
}

static inline Side opposite(Side s) {
	return (Side)(((int)s) ^ 1);
}

FinalAgent::FinalAgent(uint16_t ucbDepth, uint16_t ucbBaseGames, uint32_t iterations)
	: depth_(ucbDepth), baseGames_(ucbBaseGames), iterations_(iterations), timePerMove_(1.0), useIterations_(true)
{}

uint16_t& FinalAgent::depth() {
	return depth_;
}

uint16_t& FinalAgent::baseGames() {
	return baseGames_;
}

uint32_t& FinalAgent::iterations() {
	return iterations_;
}

float& FinalAgent::timePerMove() {
	return timePerMove_;
}

bool& FinalAgent::useIterations() {
	return useIterations_;
}

static std::tuple<uint32_t, uint32_t> montecarlo(UCB* ucbs, size_t idx, size_t baseGames, std::function<uint32_t()>& alloc);

uint8_t FinalAgent::makeMove(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	return makeMoveAndScore(b, s, movesSoFar, lastMove).first;
}

static size_t leaves = 0;

std::pair<uint8_t, float> FinalAgent::makeMoveAndScore(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
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

	if(movesSoFar > 20){
		Board bCopy = b;
		std::pair<uint8_t, double> result = minimaxAgent.iterative_deepening(s, bCopy, movesSoFar);
		if(s == SOUTH && result.second > 100){
			return std::make_pair(result.first, 1.0);
		} else if(s == NORTH && result.second < -100){
			return std::make_pair(result.first, 0.0);
		}
	}

	size_t len = neededSize(depth_);
	auto ucbs = std::unique_ptr<UCB[]>(new UCB[len]);

	size_t nMoves;
	const uint8_t* moves = b.validMoves(s, nMoves);
	assert(nMoves > 0);

	ucbs[0].board = b;
	ucbs[0].whosTurn = s;

	uint32_t cur = 0;
	auto lalloc = [&]() { return (cur + 1 < len) ? ++cur : ~0u; };
	std::function<uint32_t()> alloc = lalloc;

	leaves = 0;
	if(useIterations_) {
		for(size_t i = 0; i < iterations_; i++) {
			montecarlo(ucbs.get(), 0, baseGames_, alloc);
		}
	} else {
		auto deadline = high_resolution_clock::now() + duration<double>(timePerMove_);

		auto t1 = high_resolution_clock::now();
		montecarlo(ucbs.get(), 0, baseGames_, alloc);
		auto t2 = high_resolution_clock::now();

		size_t itsCompleted = ucbs[0].plays / 2 / baseGames_;

		while(t2 < deadline) {
			double itsPerSec = itsCompleted / duration_cast<duration<double>>(t2 - t1).count();
			itsCompleted = std::max(size_t(1), size_t(itsPerSec * duration_cast<duration<double>>(deadline - t2).count()));

			t1 = high_resolution_clock::now();
			for(size_t i = 0; i < itsCompleted; i++) {
				montecarlo(ucbs.get(), 0, baseGames_, alloc);
			}
			t2 = high_resolution_clock::now();
		}
	}

	size_t total = 0;
	for(size_t i = 0; i < len; i++) {
		if(ucbs[i].plays == 0) total++;
	}
	std::cerr << "len " << len << std::endl;
	std::cerr << "total " << total << std::endl;
	std::cerr << "maxidx " << cur << std::endl;
	std::cerr << "simulations " << leaves << std::endl;

	size_t bestMove = moves[0];
	double bestScore = -std::numeric_limits<double>::infinity();
	size_t mostPlays = 0;

	for(size_t i = 0; i < nMoves; i++) {
		size_t idx = ucbs[0].childIdxs[i];
		double score = ucbs[idx].wins[(int)s] / (double) ucbs[idx].plays;
		size_t plays = ucbs[idx].plays;

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

static std::tuple<uint32_t, uint32_t> randomPlayouts(const Board& b, Side toMove, size_t games) {
	uint32_t wins[2] = { 0 };

	for(size_t i = 0; i < games; i++) {
		g.board() = b;
		g.movesPlayed() = 3; // to avoid switching
		g.toMove() = toMove;

		while(g.board().stonesInWell(SOUTH) <= 49 && g.board().stonesInWell(NORTH) <= 49 && !g.isOver()) g.stepTurn();

		if(g.board().stonesInWell(SOUTH) > 49) {
			wins[0] += 2;
		} else if(g.board().stonesInWell(NORTH) > 49) {
			wins[1] += 2;
		} else {
			uint8_t scores[2] = { g.board().stonesInWell(SOUTH), g.board().stonesInWell(NORTH) };
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

	return std::make_tuple(wins[0], wins[1]);

}

// South score, north score
static std::tuple<uint32_t, uint32_t> montecarlo(UCB* ucbs, size_t idx, size_t baseGames, std::function<uint32_t()>& alloc) {
	UCB& cur = ucbs[idx];
	const Side toMove = cur.whosTurn;
	const Side opp = opposite(toMove);

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

	// Expansion
	if(cur.plays == 0) {
		bool ok = true;
		for(size_t i = 0; i < nMoves; i++) {
			cur.childIdxs[i] = alloc();
			if(cur.childIdxs[i] == ~0u) {
				ok = false;
				break;
			}
		}

		if(!ok) {
			for(size_t i = 0; i < nMoves; i++) {
				cur.childIdxs[i] = ~0u;
			}
		} else {
			for(size_t i = 0; i < nMoves; i++) {
				UCB& child = ucbs[cur.childIdxs[i]];

				child.board = cur.board;
				bool ga = child.board.makeMove(cur.whosTurn, moves[i]);

				child.whosTurn = ga ? toMove : opp;
			}
		}

		leaves++;
		auto res = randomPlayouts(cur.board, toMove, baseGames);
		cur.plays = 2 * baseGames;
		cur.wins[0] = std::get<0>(res);
		cur.wins[1] = std::get<1>(res);

		return res;
	}

	// Selection + backpropagation
	if(cur.childIdxs[0] != ~0u) {
		size_t total = 0;
		for(size_t i = 0; i < nMoves; i++) {
			total += ucbs[cur.childIdxs[i]].plays;
		}


		double logTotal = 3.0 * log(cur.plays);
		double max = -std::numeric_limits<double>::infinity();
		size_t moveIdx = 0;

		for(size_t i = 0; i < nMoves; i++) {
			UCB& child = ucbs[cur.childIdxs[i]];

			if(child.plays == 0) {
				moveIdx = i;
				break;
			}

			double bound = child.wins[(int)toMove] / (float) child.plays;
			bound += sqrt(logTotal / child.plays);

			if(bound > max) {
				max = bound;
				moveIdx = i;
			}
		}

		size_t childI = cur.childIdxs[moveIdx];
		uint64_t curPlays = ucbs[childI].plays;

		auto res = montecarlo(ucbs, childI, baseGames, alloc);

		cur.wins[0] += std::get<0>(res);
		cur.wins[1] += std::get<1>(res);
		cur.plays += ucbs[childI].plays - curPlays;

		return res;
	}

	// Random playouts
	leaves++;
	auto res = randomPlayouts(cur.board, cur.whosTurn, baseGames);
	cur.plays += 2 * baseGames;
	cur.wins[0] += std::get<0>(res);
	cur.wins[1] += std::get<1>(res);

	return res;
}

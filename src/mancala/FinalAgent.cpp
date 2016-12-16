#include "FinalAgent.hpp"
#include "RandomAgent.hpp"
#include "Game.hpp"

#include <cassert>
#include <random>
#include <limits>
#include <memory>
#include <cmath>
#include <tuple>
#include <chrono>
#include <unordered_map>

///////////////////////////////////////////////
//
//                   MINIMAX
//
///////////////////////////////////////////////
static std::unordered_map<Board, double> cache_north = {};
static std::unordered_map<Board, double> cache_south = {};

static inline void cacheIt(const Board& b, double val, Side s){
	Board bCopy = b;		                             
	if(s == SOUTH)
		cache_south.insert(std::make_pair(bCopy, val));
	if(s == NORTH)
		cache_north.insert(std::make_pair(bCopy, val));
}

static std::pair<uint8_t,double> iterative_deepening(Side toMove, const Board& b, size_t movesSoFar);
static std::pair<uint8_t,double> minimax_alphabeta(uint8_t depth, Side s, Board& b, size_t movesSoFar, double alpha, double beta);

static inline bool pairCompare(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem) {
  return firstElem.second > secondElem.second;
}

static inline bool pairCompare_minimize(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem) {
  return firstElem.second < secondElem.second;
}

static inline bool isSeedable(const Board& b, Side s, uint8_t idx) {
	bool toRet = false;
	int8_t cur = idx - 1;

	do {
		if(cur < 0) break;
		if(idx - cur == b.stonesInHole(s, cur)) {
			toRet = true;
			break;
		}
		cur--;
	} while(true);

	return toRet;
}

static inline double jimmy_heuristic(const Board& b, Side s) {
	Side o = Side(int(s)^1);
	double d = 0.0;

	double ourWell = b.stonesInWell(s);
	double oppWell = b.stonesInWell(o);
	if((ourWell != 0.0 || oppWell != 0.0) && ourWell != oppWell) {
		double bigWell;
		double smallWell;
		if(ourWell > oppWell) {
			bigWell = ourWell;
			smallWell = oppWell;
		} else {
			bigWell = oppWell;
			smallWell = ourWell;
		}
		d = ((1.0 / bigWell) * (bigWell - smallWell) + 1.0) * bigWell;
		if(oppWell > ourWell) d *= -1;
	}

	size_t N;
	const auto* moves = b.validMoves(s, N);

	int ourSum = 0;
	for(size_t i = 0; i < N; i++) {
		ourSum += b.stonesInHole(s, moves[i]);
	}

	int oppSum = 98 - ourWell - oppWell - ourSum;

	d += (ourSum - oppSum) / 2.0;

	for(uint8_t i = 0; i < 7; i++) {
		if(b.stonesInHole(o, i) == 0 && isSeedable(b, o, i)) {
			d -= b.stonesInHole(s, 6-i);
		}
	}

	return d;
}

static std::pair<uint8_t,double> minimax_alphabeta(uint8_t depth, const Side toMove, Board& b, size_t movesSoFar, double alpha, double beta){
	size_t nMoves;
	auto* moves = b.validMoves(toMove, nMoves);

	// The Game is Actually Over
	if(nMoves == 0){
		uint8_t scores[2] = { b.stonesInWell(SOUTH), b.stonesInWell(NORTH) };
		scores[Side(int(toMove)^1)] += 98 - scores[0] - scores[1];

		int scoreDiff = int(scores[0]) - scores[1];

		double val = scoreDiff > 0 ?  1.0/0.0 :
		             scoreDiff < 0 ? -1.0/0.0 :
		                             0.0;
		cacheIt(b, val, scoreDiff > 0? SOUTH : NORTH);
		return std::make_pair(0, val);
	}

	// Someone Can Reach A Certain Win
	if(b.stonesInWell(SOUTH) > 49){
		cacheIt(b, 1.0/0.0, SOUTH);
		return std::make_pair(moves[0], 1.0/0.0);
	}
	else if(b.stonesInWell(NORTH) > 49){
		cacheIt(b, -1.0/0.0, NORTH);
		return std::make_pair(moves[0], -1.0/0.0);
	}

	// We Have Reached the Maximum Depth
	if(depth == 0){
		double val = jimmy_heuristic(b, toMove);
		if(toMove != SOUTH) val *= -1;
		cacheIt(b, val, toMove);
		return std::make_pair(-1, val);
	}

	// MAXIMIZE
	if(toMove == SOUTH){
		std::pair<uint8_t, double> possibleMoves[8];
		for(uint8_t i = 0; i < nMoves; i++)
			possibleMoves[i] = std::make_pair(moves[i], -1.0/0.0);

		// Apply Move Reordering
		if(depth > 3){
			// Check All Moves
			for(uint8_t i = 0; i < nMoves; i++){
				if(cache_south.find(b) != cache_south.end()){
					possibleMoves[i].second = cache_south[b];
				} else{
					Board nCopy = b;
					bool ga = nCopy.makeMove(toMove, possibleMoves[i].first);
					possibleMoves[i].second = jimmy_heuristic(nCopy, ga ? SOUTH : NORTH);
					if(!ga) possibleMoves[i].second *= -1;
				}
			}

			// Sort based on best payoff
			std::sort(std::begin(possibleMoves), std::begin(possibleMoves)+nMoves, pairCompare);
		}

		std::pair<uint8_t,double> result = std::make_pair(8, -1.0/0.0);
		for(uint8_t i = 0; i < nMoves; i++){
			Board nCopy = b;
			uint8_t move = possibleMoves[i].first;
			bool goAgain = nCopy.makeMove(toMove, move);

			std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, goAgain ? SOUTH : NORTH, nCopy, movesSoFar+1, alpha, beta);
			if(nResult.second >= result.second){
				result = nResult;
				result.first = move;
				alpha = std::max(alpha, result.second);
				if(beta <= alpha){
					break;
				}
			}

		}
		cacheIt(b, result.second, SOUTH);
		return result;
	} 
	// MINIMIZE
	else {
		std::pair<uint8_t, double> possibleMoves[8];
		for(uint8_t i = 0; i <= nMoves; i++)
			possibleMoves[i] = std::make_pair(moves[i], 1.0/0.0);
		// Apply Move Reordering
		if(depth > 3){
			// Check All Moves
			for(uint8_t i = 0; i < nMoves; i++){
				if(cache_north.find(b) != cache_north.end()){
					possibleMoves[i].second = cache_north[b];
				} else{
					Board nCopy = b;
					bool ga = nCopy.makeMove(toMove, possibleMoves[i].first);
					possibleMoves[i].second = jimmy_heuristic(nCopy, ga ? NORTH : SOUTH);
					if(ga) possibleMoves[i].second *= -1;
				}
			}

			// Sort based on best payoff
			std::sort(std::begin(possibleMoves), std::begin(possibleMoves)+nMoves, pairCompare_minimize);
		}

		std::pair<uint8_t,double> result = std::make_pair(8, 1.0/0.0);
		for(uint8_t i = 0; i < nMoves; i++){
			Board nCopy = b;
			uint8_t move = possibleMoves[i].first;
			bool goAgain = nCopy.makeMove(toMove, move);

			std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, goAgain ? NORTH : SOUTH, nCopy, movesSoFar+1, alpha, beta);
			if(nResult.second <= result.second){
				result = nResult;
				result.first = move;
				beta = std::min(beta, result.second);
				if(beta <= alpha){
					break;
				}
			}

		}
		cacheIt(b, result.second, NORTH);
		return result;
	}
}

static std::pair<uint8_t,double> iterative_deepening(Side toMove, const Board& b, size_t movesSoFar){
	uint8_t MAX_DEPTH = 7;
	std::pair<uint8_t,double> final_result = std::make_pair(8, toMove == SOUTH? -1.0/0.0 : 1.0/0.0);

	for(uint8_t depth = 1; depth < MAX_DEPTH; depth++){
		Board bCopy = b;
		final_result = minimax_alphabeta(depth, toMove, bCopy, movesSoFar, -1.0/0.0, 1.0/0.0);
	}
	return final_result;
}
///////////////////////////////////////////////
//
//                   MCTS
//
///////////////////////////////////////////////
struct UCB {
	Board board;
	uint64_t plays = 0;
	uint64_t wins[2] = { 0 };
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

static std::tuple<uint32_t, uint32_t> montecarlo(UCB* ucbs, size_t depth, size_t idx, size_t baseGames);

uint8_t FinalAgent::makeMove(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	return makeMoveAndScore(b, s, movesSoFar, lastMove).first;
}

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
		std::pair<uint8_t,double> result = iterative_deepening(s, bCopy, movesSoFar);
		if(s == SOUTH && result.second > 100){
			return std::make_pair(result.first, 1.0);
		} else if(s == NORTH && result.second < -100){
			return std::make_pair(result.first, -1.0);
		}
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
		double score = ucbs[moves[i] + 1].wins[(int)s] / (double) ucbs[moves[i] + 1].plays;
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
	UCB& cur = ucbs[idx];
	Side toMove = cur.whosTurn;
	Side opp = opposite(toMove);

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

	if(depth == 0) {
		uint32_t wins[2] = { 0 };

		for(size_t i = 0; i < baseGames; i++) {
			g.board() = cur.board;
			g.movesPlayed() = 3; // to avoid switching
			g.toMove() = toMove;

			while(g.board().stonesInWell(SOUTH) <= 49 && g.board().stonesInWell(NORTH) <= 49 && !g.isOver()) g.stepTurn();
			
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

		return std::make_tuple(wins[0], wins[1]);
	}
	
	// Populate children if necessary
	if(cur.plays == 0) {
		uint32_t wins[2] = { 0 };
		for(size_t i = 0; i < nMoves; i++) {
			uint8_t move = moves[i];
			UCB& child = ucbs[childIdx(idx, move)];

			child.board = cur.board;
			bool ga = child.board.makeMove(toMove, move);

			child.whosTurn = ga ? toMove : opp;

			auto res = montecarlo(ucbs, depth - 1, childIdx(idx, move), baseGames);

			cur.plays += child.plays;
			wins[0] += std::get<0>(res);
			wins[1] += std::get<1>(res);

		}
		cur.wins[0] += wins[0];
		cur.wins[1] += wins[1];

		return std::make_tuple(wins[0], wins[1]);
	}
	
	// go by max bound otherwise
	double logTotal = 2.0 * log(cur.plays);
	double max = -std::numeric_limits<double>::infinity();
	size_t move = moves[0];

	for(size_t i = 0; i < nMoves; i++) {
		UCB& child = ucbs[childIdx(idx, moves[i])];
		
		double bound = child.wins[(int)toMove] / (float) child.plays;
		bound += sqrt(logTotal / child.plays);

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

	return res;
}

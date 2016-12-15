#include "MiniMaxAgent.hpp"

#include <utility>
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <cassert>
#include <vector>
#include <algorithm>
#include <unordered_map>

static std::unordered_map<Board, double> cache = {};

static std::pair<uint8_t,double> minimax_alphabeta(uint8_t depth, Side s, Board& b, size_t movesSoFar, double alpha, double beta);

static bool pairCompare(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem);
static bool pairCompare_minimize(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem);
static uint8_t iterative_deepening(Side toMove, const Board& b, size_t movesSoFar);
uint8_t MiniMaxAgent::makeMove(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	// Swap Logic
	if(movesSoFar == 0) return 0;
	if(movesSoFar == 1 && (lastMove == 1 || lastMove == 3 || lastMove == 4 || lastMove == 5 || lastMove == 6)) return 7;

	size_t nMoves;
	auto* moves = b.validMoves(s, nMoves);
	assert(nMoves > 0);

	// The game is already over
	if(b.stonesInWell(s) > 49)
		return moves[rand() % nMoves];

	Board bCopy = b;
	return iterative_deepening(s, bCopy, movesSoFar);
}

static bool pairCompare(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem) {
  return firstElem.second > secondElem.second;
}

static void cacheIt(const Board& b, double val){
	Board bCopy = b;		                             
	cache.insert(std::make_pair(bCopy, val));
}

static bool pairCompare_minimize(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem) {
  return firstElem.second < secondElem.second;
}

static uint8_t iterative_deepening(Side toMove, const Board& b, size_t movesSoFar){
	uint8_t MAX_DEPTH = 4;
	std::pair<uint8_t,double> final_result = std::make_pair(8, toMove == SOUTH? -1.0/0.0 : 1.0/0.0);

	for(uint8_t depth = 1; depth < MAX_DEPTH; depth++){
		Board bCopy = b;
		final_result = minimax_alphabeta(depth, toMove, bCopy, movesSoFar, -1.0/0.0, 1.0/0.0);
	}
	return final_result.first;
}

/// Returns the heuristic value for south. 0 indicates a draw, positive values an advantage for south, and negative values and advantage for north.
static inline double heuristic(const Board& b) {
	return double(b.stonesInWell(SOUTH)) - b.stonesInWell(NORTH);
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
		cacheIt(b, val);
		return std::make_pair(0, val);
	}

	// Someone Can Reach A Certain Win
	if(b.stonesInWell(SOUTH) > 49){
		cacheIt(b, 1.0/0.0);
		return std::make_pair(moves[0], 1.0/0.0);
	}
	else if(b.stonesInWell(NORTH) > 49){
		cacheIt(b, -1.0/0.0);
		return std::make_pair(moves[0], -1.0/0.0);
	}

	// We Have Reached the Maximum Depth
	if(depth == 0){
		double val = heuristic(b);
		if(toMove != SOUTH) val *= -1;
		cacheIt(b, val);
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
				if(cache.find(b) != cache.end()){
					possibleMoves[i].second = cache[b];
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
		cacheIt(b, result.second);
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
				if(cache.find(b) != cache.end()){
					possibleMoves[i].second = cache[b];
				} else{
					Board nCopy = b;
					bool ga = nCopy.makeMove(toMove, possibleMoves[i].first);
					possibleMoves[i].second = jimmy_heuristic(nCopy, ga ? NORTH : SOUTH);
					if(!ga) possibleMoves[i].second *= -1;
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
		cacheIt(b, result.second);
		return result;
	}
}

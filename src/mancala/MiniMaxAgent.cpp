#include "MiniMaxAgent.hpp"

#include <utility>
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <cassert>
#include <vector>
#include <algorithm>
#include <chrono>

static std::pair<uint8_t,double> minimax_alphabeta(uint8_t depth, Side s, Board& b, size_t movesSoFar, double alpha, double beta,
													MiniMaxAgent::MoveCache& cache_north, MiniMaxAgent::MoveCache& cache_south);
std::pair<uint8_t,double> iterative_deepening(Side toMove, const Board& b, size_t movesSoFar, double time);

uint8_t MiniMaxAgent::makeMove(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	// Swap Logic
	if(movesSoFar == 0) return 1;
	if(movesSoFar == 1 && (lastMove == 1 || lastMove == 2 ||  lastMove == 3 || lastMove == 4 || lastMove == 5 || lastMove == 6)) return 7;

	size_t nMoves;
	auto* moves = b.validMoves(s, nMoves);
	assert(nMoves > 0);

	// The game is already over
	if(b.stonesInWell(s) > 49 || b.stonesInWell(Side(int(s)^1)) > 49)
		return moves[rand() % nMoves];

	Board bCopy = b;
	std::function<void(uint8_t, double)> ff = [](uint8_t, double) {};

	return iterative_deepening(s, bCopy, movesSoFar, 10.0, ff).first;
}

static inline bool pairCompare(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem) {
  return firstElem.second > secondElem.second;
}

static inline bool pairCompare_minimize(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem) {
  return firstElem.second < secondElem.second;
}

static inline void cacheIt(const Board& b, double val, Side s, MiniMaxAgent::MoveCache& cache_north, MiniMaxAgent::MoveCache& cache_south){
	Board bCopy = b;		                             
	if(s == SOUTH)
		cache_south.insert(std::make_pair(bCopy, val));
	if(s == NORTH)
		cache_north.insert(std::make_pair(bCopy, val));
}

std::pair<uint8_t,double> MiniMaxAgent::iterative_deepening(Side toMove, const Board& b,
																size_t movesSoFar, double time, std::function<void(uint8_t, double)> up){
	MiniMaxAgent::MoveCache cache_north = {};
	MiniMaxAgent::MoveCache cache_south = {};

	uint8_t CURRENT_DEPTH = 6;
	std::pair<uint8_t,double> final_result = std::make_pair(8, toMove == SOUTH? -1.0/0.0 : 1.0/0.0);

	auto current = std::chrono::high_resolution_clock::now();
	auto deadline = current + std::chrono::duration<double>(time);
	
	while(current < deadline){
		Board bCopy = b;
		final_result = minimax_alphabeta(CURRENT_DEPTH, toMove, bCopy, movesSoFar, -1.0/0.0, 1.0/0.0, cache_north, cache_south);
		
		up(final_result.first, final_result.second);

		CURRENT_DEPTH++;
		current = std::chrono::high_resolution_clock::now();
	}

	std::cout << (int) CURRENT_DEPTH << std::endl;
	return final_result;
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

static std::pair<uint8_t,double> minimax_alphabeta(uint8_t depth, const Side toMove, Board& b, size_t movesSoFar, double alpha,	
													double beta, MiniMaxAgent::MoveCache& cache_north, MiniMaxAgent::MoveCache& cache_south){
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
		cacheIt(b, val, scoreDiff > 0? SOUTH : NORTH, cache_north, cache_south);
		return std::make_pair(0, val);
	}

	// Someone Can Reach A Certain Win
	if(b.stonesInWell(SOUTH) > 49){
		cacheIt(b, 1.0/0.0, SOUTH, cache_north, cache_south);
		return std::make_pair(moves[0], 1.0/0.0);
	}
	else if(b.stonesInWell(NORTH) > 49){
		cacheIt(b, -1.0/0.0, NORTH, cache_north, cache_south);
		return std::make_pair(moves[0], -1.0/0.0);
	}

	// We Have Reached the Maximum Depth
	if(depth == 0){
		double val = jimmy_heuristic(b, toMove);
		if(toMove != SOUTH) val *= -1;
		cacheIt(b, val, toMove, cache_north, cache_south);
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

			std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, goAgain ? SOUTH : NORTH, nCopy,
																	 movesSoFar+1, alpha, beta, cache_north, cache_south);
			if(nResult.second >= result.second){
				result = nResult;
				result.first = move;
				alpha = std::max(alpha, result.second);
				if(beta <= alpha){
					break;
				}
			}

		}
		cacheIt(b, result.second, SOUTH, cache_north, cache_south);
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

			std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, goAgain ? NORTH : SOUTH, nCopy, 
																	movesSoFar+1, alpha, beta, cache_north, cache_south);
			if(nResult.second <= result.second){
				result = nResult;
				result.first = move;
				beta = std::min(beta, result.second);
				if(beta <= alpha){
					break;
				}
			}

		}
		cacheIt(b, result.second, NORTH, cache_north, cache_south);
		return result;
	}
}

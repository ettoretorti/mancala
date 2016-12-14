#include "MiniMaxAgent.hpp"

#include <utility>
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <cassert>
#include <vector>
#include <algorithm>

static std::pair<uint8_t,double> minimax_alphabeta(uint8_t depth, Side s, Board& b, size_t movesSoFar, double alpha, double beta);

static bool pairCompare(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem);
static bool pairCompare_minimize(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem);

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

	Side toMove = s;
	uint8_t depth = 11;
	Board bCopy = b;

	return minimax_alphabeta(depth, toMove, bCopy, movesSoFar, -1.0/0.0, 1.0/0.0).first;
}

static bool pairCompare(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem) {
  return firstElem.first > secondElem.first;
}

static bool pairCompare_minimize(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem) {
  return firstElem.first < secondElem.first;
}

static std::pair<uint8_t,double> minimax_alphabeta(uint8_t depth, Side toMove, Board& b, size_t movesSoFar, double alpha, double beta){
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
		return std::make_pair(0, val);
	}

	// Someone Can Reach A Certain Win
	if(b.stonesInWell(SOUTH) > 49)
		return std::make_pair(0, 1.0/0.0);
	else if(b.stonesInWell(NORTH) > 49)
		return std::make_pair(0, -1.0/0.0);

	// We Have Reached the Maximum Depth
	if(depth == 0){
		double val = (double)b.stonesInWell(SOUTH) - (double)b.stonesInWell(NORTH);
		return std::make_pair(0, val);
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
				Board nCopy = b;
				nCopy.makeMove(toMove, possibleMoves[i].first);
				possibleMoves[i].second = (double)nCopy.stonesInWell(SOUTH) - (double)nCopy.stonesInWell(NORTH);
			}

			// Sort based on best payoff
			std::sort(std::begin(possibleMoves), std::begin(possibleMoves)+nMoves, pairCompare);
		}

		std::pair<uint8_t,double> result = std::make_pair(8, -1.0/0.0);
		for(uint8_t i = 0; i < nMoves; i++){
			Board nCopy = b;
			uint8_t move = possibleMoves[i].first;
			bool goAgain = nCopy.makeMove(toMove, move);
			if(!goAgain) toMove = NORTH;

			std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, toMove, nCopy, movesSoFar+1, alpha, beta);
			if(nResult.second >= result.second){
				result = nResult;
				result.first = move;
				alpha = result.second;
				if(beta <= alpha){
					break;
				}
			}

		}
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
				Board nCopy = b;
				nCopy.makeMove(toMove, possibleMoves[i].first);
				possibleMoves[i].second = (double)nCopy.stonesInWell(SOUTH) - (double)nCopy.stonesInWell(NORTH);
			}

			// Sort based on best payoff
			std::sort(std::begin(possibleMoves), std::begin(possibleMoves)+nMoves, pairCompare_minimize);
		}

		std::pair<uint8_t,double> result = std::make_pair(8, 1.0/0.0);
		for(uint8_t i = 0; i < nMoves; i++){
			Board nCopy = b;
			uint8_t move = possibleMoves[i].first;
			bool goAgain = nCopy.makeMove(toMove, move);
			if(!goAgain) toMove = SOUTH;

			std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, toMove, nCopy, movesSoFar+1, alpha, beta);
			if(nResult.second <= result.second){
				result = nResult;
				result.first = move;
				alpha = result.second;
				if(beta <= alpha){
					break;
				}
			}

		}
		return result;
	}
}

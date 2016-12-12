#include "MiniMaxAgent.hpp"

#include <utility>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>

static std::pair<uint8_t,double> minimax_alphabeta(uint8_t depth, Side s, Side yourSide, Board& b, size_t movesSoFar, double alpha, double beta, uint8_t goAgainsNum);

static bool pairCompare(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem);
static bool pairCompare_minimize(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem);

uint8_t MiniMaxAgent::makeMove(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	size_t nMoves;
	auto* moves = b.validMoves(s, nMoves);
	assert(nMoves > 0);
	if(b.stonesInWell(s) > 49)
		return moves[rand() % 6];
	Side toMove = s;
	Side yourSide = s;
	uint8_t depth = 12;
	Board bCopy = b;

	std::pair<uint8_t,double> result = minimax_alphabeta(depth, toMove, yourSide, bCopy, movesSoFar, -1.0/0.0, 1.0/0.0, 0);
	return moves[result.first];
}

static bool pairCompare(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem) {
  return firstElem.first > secondElem.first;
}

static bool pairCompare_minimize(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem) {
  return firstElem.first < secondElem.first;
}

static std::pair<uint8_t,double> minimax_alphabeta(uint8_t depth, Side s, Side yourSide, Board& b, size_t movesSoFar, double alpha, double beta, uint8_t goAgainsNum){
	Side opponentSide = (Side)((int)yourSide ^ 1);
	Side toMove = s;
	size_t nMoves;
	auto* moves = b.validMoves(toMove, nMoves);

	// The Game is Actually Over
	if(nMoves == 0){
		uint8_t scores[2] = { b.stonesInWell(SOUTH), b.stonesInWell(NORTH) };
		scores[Side(int(s)^1)] += 98 - scores[0] - scores[1];

		int scoreDiff = int(scores[yourSide]) - scores[opponentSide];

		double val = scoreDiff > 0 ?  1.0/0.0 :
		             scoreDiff < 0 ? -1.0/0.0 :
		                             0.0;
		return std::make_pair(0, val);
	}

	// Someone Can Reach A Certain Win
	if(b.stonesInWell(yourSide) > 49)
		return std::make_pair(0, 1.0/0.0);
	else if(b.stonesInWell(opponentSide) > 49)
		return std::make_pair(0, -1.0/0.0);

	// We Have Reached the Maximum Depth
	if(depth == 0)
		return std::make_pair(0, b.stonesInWell(yourSide) - b.stonesInWell(opponentSide));
	
	// MAXIMIZE
	if(toMove == yourSide){
		std::pair<uint8_t, double> possibleMoves[8];
		for(uint8_t i = 0; i < nMoves; i++)
			possibleMoves[i] = std::make_pair(moves[i], -1.0/0.0);
		possibleMoves[nMoves] = std::make_pair(7, -1.0/0.0);

		// Apply Move Reordering
		if(depth > 3){
			// Check Swap
			if(movesSoFar == 1)
				possibleMoves[nMoves].second = b.stonesInWell(opponentSide) - b.stonesInWell(yourSide);
			// Check All Moves
			for(uint8_t i = 0; i < nMoves; i++){
				Board nCopy = b;
				bool goAgain = nCopy.makeMove(toMove, possibleMoves[i].first);
				possibleMoves[i].second = nCopy.stonesInWell(yourSide) - nCopy.stonesInWell(opponentSide);
			}

			// Sort based on best payoff
			std::sort(std::begin(possibleMoves), std::begin(possibleMoves)+nMoves+1, pairCompare);
		}

		std::pair<uint8_t,double> result = std::make_pair(8, -1.0/0.0);
		for(uint8_t i = 0; i < nMoves+1; i++){
			Board nCopy = b;
			uint8_t move = possibleMoves[i].first;
			/*
			* If move equal seven and movesSoFar is One, 
			* I would have 99 problems, but a switch ain't one
			*/
			if(move == 7 && movesSoFar == 1){
				std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, toMove, opponentSide, nCopy, movesSoFar+1, alpha, beta, 0);
				if(nResult.second >= result.second){
					result = nResult;
					result.first = possibleMoves[i].first;
					alpha = result.second;
					if(beta <= alpha){
						break;
					}
				}
			}
			else if(move < 7){
				bool goAgain = nCopy.makeMove(toMove, move);
				if(!goAgain) 
					toMove = (Side)((int)toMove ^ 1);
				std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, toMove, yourSide, nCopy, movesSoFar+1, alpha, beta, 0);
				if(nResult.second >= result.second){
					result = nResult;
					result.first = possibleMoves[i].first;
					alpha = result.second;
					if(beta <= alpha){
						break;
					}
				}
			}
			else
				assert(false);
		}
		return result;
	} 
	// MINIMIZE
	else {
		std::pair<uint8_t, double> possibleMoves[8];
		for(uint8_t i = 0; i < nMoves; i++)
			possibleMoves[i] = std::make_pair(moves[i], 1.0/0.0);
		possibleMoves[nMoves] = std::make_pair(7, 1.0/0.0);

		// Apply Move Reordering
		if(depth > 3){
			// Check Swap
			if(movesSoFar == 1)
				possibleMoves[nMoves].second = b.stonesInWell(opponentSide) - b.stonesInWell(yourSide);
			// Check All Moves
			for(uint8_t i = 0; i < nMoves; i++){
				Board nCopy = b;
				bool goAgain = nCopy.makeMove(toMove, possibleMoves[i].first);
				possibleMoves[i].second = nCopy.stonesInWell(yourSide) - nCopy.stonesInWell(opponentSide);
			}
			// Sort based on best payoff
			std::sort(std::begin(possibleMoves), std::begin(possibleMoves)+nMoves+1, pairCompare_minimize);
		}

		std::pair<uint8_t,double> result = std::make_pair(0, 1.0/0.0);
		for(uint8_t i = 0; i < nMoves+1; i++){
			Board nCopy = b;
			uint8_t move = possibleMoves[i].first;
			if(move == 7 && movesSoFar == 1){
				std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, toMove, opponentSide, nCopy, movesSoFar+1, alpha, beta, 0);
				if(nResult.second <= result.second){
					result = nResult;
					result.first = possibleMoves[i].first;
					beta = result.second;
					if(beta <= alpha){
						break;
					}
				}
			}
			else if(move < 7){
				bool goAgain = nCopy.makeMove(toMove, move);
				if(!goAgain)
					toMove = (Side)((int)toMove ^ 1);
				std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, toMove, yourSide, nCopy, movesSoFar+1, alpha, beta, 0);
				if(nResult.second <= result.second){
					result = nResult;
					result.first = possibleMoves[i].first;
					beta = result.second;
					if(beta <= alpha){
						break;
					}
				}
			}
			else
				assert(false);
		}
		return result;
	}
}

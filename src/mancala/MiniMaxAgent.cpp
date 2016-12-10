#include "MiniMaxAgent.hpp"

#include <utility>
#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>

/**
*	index: The index of the best move. If the index == 7, then its a switch
*/
static std::pair<uint8_t,double> minimax_alphabeta(uint8_t depth, Side s, Side yourSide, Board& b, size_t movesSoFar, double alpha, double beta, uint8_t goAgainsNum);

static bool pairCompare(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem);
static bool pairCompare_minimize(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem);

uint8_t MiniMaxAgent::makeMove(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	size_t nMoves;
	auto* moves = b.validMoves(s, nMoves);
	assert(nMoves > 0);

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
	Side toMove = s;
	size_t nMoves;
	auto* moves = b.validMoves(toMove, nMoves);

	if(nMoves == 0){
		// TODO: add stones in board
		double totalYourSide = b.stonesInWell(yourSide) - b.stonesInWell((Side)((int)yourSide ^ 1));
		if(totalYourSide > 0)
			return std::make_pair(0, 1.0/0.0);
		else if(totalYourSide < 0)
			return std::make_pair(0, -1.0/0.0);
		else
			return std::make_pair(0, 0.0);
	}
	if(depth == 0)
		return std::make_pair(0, b.stonesInWell(yourSide) - b.stonesInWell((Side)((int)yourSide ^ 1)));
	
	// MAXIMIZE
	if(toMove == yourSide){
		std::pair<uint8_t, double> possibleMoves[8];
		for(uint8_t i = 0; i < nMoves; i++)
			possibleMoves[i] = std::make_pair(moves[i], -1.0/0.0);
		possibleMoves[nMoves] = std::make_pair(7, -1.0/0.0);

		if(depth > 3){
			// HARDCODE swap
			if(movesSoFar == 1){
				yourSide = (Side)((int)yourSide ^ 1);
				possibleMoves[nMoves].second = b.stonesInWell(yourSide) - b.stonesInWell((Side)((int)yourSide ^ 1));
				yourSide = (Side)((int)yourSide ^ 1);
			}
			// Check other moves
			for(uint8_t i = 0; i < nMoves; i++){
				Board nCopy = b;
				bool goAgain = nCopy.makeMove(toMove, possibleMoves[i].first);
				possibleMoves[i].second = nCopy.stonesInWell(yourSide) - nCopy.stonesInWell((Side)((int)yourSide ^ 1));
			}

			// Sort based on best payoff
			// TODO don't consider bad moves
			std::sort(std::begin(possibleMoves), std::end(possibleMoves), pairCompare);
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
				yourSide = (Side)((int)yourSide ^ 1);
				std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, toMove, yourSide, nCopy, movesSoFar+1, alpha, beta, 0);
				if(nResult.second >= result.second){
					result = nResult;
					result.first = possibleMoves[i].first;
					alpha = result.second;
					if(beta <= alpha){
						break;
					}
				}
				yourSide = (Side)((int)yourSide ^ 1);
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
	// MINIMIZE
	} else {
		std::pair<uint8_t, double> possibleMoves[8];
		for(uint8_t i = 0; i < nMoves; i++)
			possibleMoves[i] = std::make_pair(moves[i], 1.0/0.0);
		possibleMoves[nMoves] = std::make_pair(7, 1.0/0.0);

		if(depth > 3){
			if(movesSoFar == 1){
				yourSide = (Side)((int)yourSide ^ 1);
				possibleMoves[nMoves].second = b.stonesInWell(yourSide) - b.stonesInWell((Side)((int)yourSide ^ 1));
				yourSide = (Side)((int)yourSide ^ 1);
			}
			// Check other moves
			for(uint8_t i = 0; i < nMoves; i++){
				Board nCopy = b;
				bool goAgain = nCopy.makeMove(toMove, possibleMoves[i].first);
				possibleMoves[i].second = nCopy.stonesInWell(yourSide) - nCopy.stonesInWell((Side)((int)yourSide ^ 1));
			}

			// Sort based on best payoff
			std::sort(std::begin(possibleMoves), std::end(possibleMoves), pairCompare);
		}
		std::pair<uint8_t,double> result = std::make_pair(0, 1.0/0.0);
		for(uint8_t i = 0; i < nMoves+1; i++){
			Board nCopy = b;
			uint8_t move = possibleMoves[i].first;
			if(move == 7 && movesSoFar == 1){
				yourSide = (Side)((int)yourSide ^ 1);
				std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, toMove, yourSide, nCopy, movesSoFar+1, alpha, beta, 0);
				if(nResult.second <= result.second){
					result = nResult;
					result.first = possibleMoves[i].first;
					beta = result.second;
					if(beta <= alpha){
						break;
					}
				}
				yourSide = (Side)((int)yourSide ^ 1);
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

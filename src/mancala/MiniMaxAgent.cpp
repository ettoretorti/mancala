#include "MiniMaxAgent.hpp"

#include <utility>
#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>

/**
*
*	index: The index of the best move. If the index == 7, then its a switch
*/
static std::pair<uint8_t,double> minimax_alphabeta(uint8_t depth, Side s, Side yourSide, Board& b, size_t movesSoFar, double alpha, double beta, uint8_t goAgainsNum);
static void shallowSearch(uint8_t depth, Side s, Side yourSide, Board& b, size_t startMove, std::pair<uint8_t, double> possibleMoves[], size_t movesSoFar);

static bool pairCompare(const std::pair<uint8_t, double>& firstElem, const std::pair<uint8_t, double>& secondElem);
static double scoreDiffHeuristic(Side yourSide, Board& b);
static double stonesInFrontEmptyHolesHeuristic(Side yourSide, Board& b);
static double heuristic_calc(Side yourSide, Board& b);
static int stoneNumDiff(Side yourSide, Board& b);
static uint8_t overflow(Side yourSide, Board& b);
static double parameters[4] = {4,1,1,1};
static uint8_t sDepth = 5;

uint8_t MiniMaxAgent::makeMove(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	size_t nMoves;
	auto* moves = b.validMoves(s, nMoves);
	assert(nMoves > 0);

	Side toMove = s;
	Side yourSide = s;
	uint8_t depth = 9;
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

static uint8_t overflow(Side yourSide, Board& b){
	uint8_t total = 0;
	for(int i = 0; i < 7; i++){
		uint8_t stonesLeft = b.stonesInHole(yourSide, i);
		uint8_t currentHole = i;
		while(stonesLeft > 0){
			stonesLeft -= (7-currentHole);
			if(stonesLeft > 6){
				total += 6;
				stonesLeft -= 6;
				currentHole = 0;
			} else if(stonesLeft > 0){
				total += stonesLeft;
				stonesLeft = 0;
			}
		}
	}
	return total;
}

static double stonesInFrontEmptyHolesHeuristic(Side yourSide, Board& b){
	double total = 0.0;
	for(int i = 0; i < 7; i++){
		if(b.stonesInHole(yourSide, i) == 0){
			total += b.stonesInHole((Side)((int)yourSide ^ 1), i);
		}
		if(b.stonesInHole((Side)((int)yourSide ^ 1), i) == 0){
			total -= b.stonesInHole(yourSide, i);
		}
	}
	return total;
}

static int stoneNumDiff(Side yourSide, Board& b){
	return b.totalStones(yourSide) - b.totalStones((Side)((int)yourSide ^ 1));
}

static double scoreDiffHeuristic(Side yourSide, Board& b) {
	return b.stonesInWell(yourSide) - b.stonesInWell((Side)((int)yourSide ^ 1));
}

static double heuristic_calc(Side yourSide, Board& b){
	return parameters[0]*scoreDiffHeuristic(yourSide, b) + parameters[1]*stoneNumDiff(yourSide, b) + parameters[2]*stonesInFrontEmptyHolesHeuristic(yourSide, b) - parameters[3]*overflow(yourSide, b);
}

static void shallowSearch(uint8_t depth, Side s, Side yourSide, Board& b, size_t startMove, std::pair<uint8_t, double> possibleMoves[], size_t movesSoFar){
	Side toMove = s;
	size_t nMoves;
	auto* moves = b.validMoves(toMove, nMoves);
	if(nMoves == 0){
		double totalYourSide = heuristic_calc(yourSide, b);
		double totalOtherSide = heuristic_calc((Side)((int)yourSide ^ 1), b);
		if(totalYourSide > totalOtherSide)
			possibleMoves[startMove] = std::make_pair(startMove, 1.0/0.0);
		else if(totalYourSide == totalOtherSide && possibleMoves[startMove].second < 0.0)
			possibleMoves[startMove] = std::make_pair(startMove,0.0);
		return;
	}
	if(depth == 0){
		double heuristic = heuristic_calc(yourSide, b);
		if(possibleMoves[startMove].second < heuristic)
			possibleMoves[startMove] = std::make_pair(startMove, heuristic);
		return;
	}
	// MAXIMIZE
	if(toMove == yourSide){
		// switch, maximum
		if(movesSoFar == 1){
			Board bCopy = b;
			yourSide = (Side)((int)yourSide ^ 1);
			shallowSearch(depth-1, toMove, yourSide, bCopy, startMove, possibleMoves, movesSoFar+1);
			yourSide = (Side)((int)yourSide ^ 1);
		}

		for(uint8_t i = 0; i < nMoves; i++) {
			Board bCopy = b;
			bool goAgain = bCopy.makeMove(toMove, moves[i]);
			if(!goAgain)
				toMove = (Side)((int)toMove ^ 1);
			shallowSearch(depth-1, toMove, yourSide, bCopy, startMove, possibleMoves, movesSoFar+1);
		}
	// MINIMIZE
	} else {
		// switch, minimum
		if(movesSoFar == 1){
			Board bCopy = b;
			yourSide = (Side)((int)yourSide ^ 1);
			shallowSearch(depth-1, toMove, yourSide, bCopy, startMove, possibleMoves, movesSoFar+1);
			yourSide = (Side)((int)yourSide ^ 1);
		}
		for(uint8_t i = 0; i < nMoves; i++) {
			Board bCopy = b;
			bool goAgain = bCopy.makeMove(toMove, moves[i]);
			if(!goAgain)
				toMove = (Side)((int)toMove ^ 1);
			shallowSearch(depth-1, toMove, yourSide, bCopy, startMove, possibleMoves, movesSoFar+1);
		}
	}
}

static std::pair<uint8_t,double> minimax_alphabeta(uint8_t depth, Side s, Side yourSide, Board& b, size_t movesSoFar, double alpha, double beta, uint8_t goAgainsNum){
	Side toMove = s;
	size_t nMoves;
	b.validMoves(toMove, nMoves);

	if(nMoves == 0){
		double totalYourSide = heuristic_calc(yourSide, b);
		double totalOtherSide = heuristic_calc((Side)((int)yourSide ^ 1), b);
		if(totalYourSide > totalOtherSide)
			return std::make_pair(0, 1.0/0.0);
		else if(totalYourSide < totalOtherSide)
			return std::make_pair(0, -1.0/0.0);
		else
			return std::make_pair(0, 0.0);
	}
	if(depth == 0){
		double totalYourSide = heuristic_calc(yourSide, b);
		return std::make_pair(0, totalYourSide);
	}
	// MAXIMIZE
	if(toMove == yourSide){
		Board nCopy = b;
		std::pair<uint8_t, double> possibleMoves[8];
		for(uint8_t i = 0; i < 8; i++)
			possibleMoves[i] = std::make_pair(8, -1.0/0.0);

		if(movesSoFar == 1){
			yourSide = (Side)((int)yourSide ^ 1);
			double totalYourSide = heuristic_calc(yourSide, b);
			possibleMoves[7] = std::make_pair(7, totalYourSide);
			yourSide = (Side)((int)yourSide ^ 1);
		}
		// Check other moves
		for(uint8_t i = 0; i < 6; i++){
			Board nCopy = b;
			bool goAgain = nCopy.makeMove(toMove, i);
			double totalYourSide = heuristic_calc(yourSide, b);
			possibleMoves[i] = std::make_pair(i, totalYourSide);
		}

		// Sort based on best payoff
		std::sort(std::begin(possibleMoves), std::end(possibleMoves), pairCompare);

		std::pair<uint8_t,double> result = std::make_pair(0, -1.0/0.0);
		for(uint8_t i = 0; i < 7; i++){
			Board nCopy = b;
			uint8_t move = possibleMoves[i].first;
			if(move != 8){
				if(move == 7 && movesSoFar == 1){
					yourSide = (Side)((int)yourSide ^ 1);
					std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, toMove, yourSide, nCopy, movesSoFar+1, -1.0/0.0, 1.0/0.0, 0);
					if(nResult.second > result.second){
						result = nResult;
						result.first = possibleMoves[i].first;
						alpha = result.second;
					}
					yourSide = (Side)((int)yourSide ^ 1);
				}
				else{
					bool goAgain = nCopy.makeMove(toMove, move);
					if(!goAgain)
						toMove = (Side)((int)toMove ^ 1);
					std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, toMove, yourSide, nCopy, movesSoFar+1, -1.0/0.0, 1.0/0.0, 0);
					if(nResult.second > result.second){
						result = nResult;
						result.first = possibleMoves[i].first;
						alpha = result.second;
					}
				}
			}
		}
		return result;
	// MINIMIZE
	} else {
		Board nCopy = b;

		std::pair<uint8_t, double> possibleMoves[8];
		for(uint8_t i = 0; i < 8; i++)
			possibleMoves[i] = std::make_pair(8, 1.0/0.0);

		if(movesSoFar == 1){
			yourSide = (Side)((int)yourSide ^ 1);
			double totalYourSide = heuristic_calc(yourSide, b);
			possibleMoves[7] = std::make_pair(7, totalYourSide);
			yourSide = (Side)((int)yourSide ^ 1);
		}
		// Check other moves
		for(uint8_t i = 0; i < 7; i++){
			Board nCopy = b;
			bool goAgain = nCopy.makeMove(toMove, i);
			double totalYourSide = heuristic_calc(yourSide, b);
			possibleMoves[i] = std::make_pair(i, totalYourSide);
		}

		// Sort based on best payoff
		std::sort(std::begin(possibleMoves), std::end(possibleMoves), pairCompare_minimize);

		std::pair<uint8_t,double> result = std::make_pair(0, 1.0/0.0);
		for(uint8_t i = 0; i < 8; i++){
			Board nCopy = b;
			uint8_t move = possibleMoves[i].first;
			if(move  != 8){
				if(move == 7 && movesSoFar == 1){
					yourSide = (Side)((int)yourSide ^ 1);
					std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, toMove, yourSide, nCopy, movesSoFar+1, -1.0/0.0, 1.0/0.0, 0);
					if(nResult.second < result.second){
						result = nResult;
						result.first = possibleMoves[i].first;
						beta = result.second;
					}
					yourSide = (Side)((int)yourSide ^ 1);
				}
				else{
					bool goAgain = nCopy.makeMove(toMove, move);
					if(!goAgain)
						toMove = (Side)((int)toMove ^ 1);
					std::pair<uint8_t,double> nResult = minimax_alphabeta(depth-1, toMove, yourSide, nCopy, movesSoFar+1, -1.0/0.0, 1.0/0.0, 0);
					if(nResult.second < result.second){
						result = nResult;
						result.first = possibleMoves[i].first;
						beta = result.second;
					}
				}
			}
		}
		return result;
	}
}

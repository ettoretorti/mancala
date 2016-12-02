#include "MiniMaxAgent.hpp"
#include <utility>
#include <iostream>
#include <cassert>

/**
*
*	index: The index of the best move. If the index == 7, then its a switch
*/
static std::pair<uint8_t,double> minimax(uint8_t depth, Side s, Side yourSide, Board& b, size_t movesSoFar);
static std::pair<uint8_t,double> minimax_alphabeta(uint8_t depth, Side s, Side yourSide, Board& b, size_t movesSoFar, double alpha, double beta, uint8_t goAgainsNum);

static double scoreDiffHeuristic(Side yourSide, Board& b);
static double stonesInFrontEmptyHolesHeuristic(Side yourSide, Board& b);
static int stoneNumDiff(Side yourSide, Board& b);
static uint8_t overflow(Side yourSide, Board& b);
double parameters[4] = {1,1,1,1};


uint8_t MiniMaxAgent::makeMove(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	size_t nMoves;
	auto* moves = b.validMoves(s, nMoves);
	assert(nMoves > 0);

	uint8_t depth = 10; // SOME DEPTH
	Board bCopy = b;
	std::pair<uint8_t,double> result = minimax_alphabeta(depth, s, s, bCopy, movesSoFar, -1.0, 96.0, 0);
	//std::cout << (int) s << ": " << (int) result.first << std::endl;
	return moves[result.first];
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

static std::pair<uint8_t,double> minimax_alphabeta(uint8_t depth, Side s, Side yourSide, Board& b, size_t movesSoFar, double alpha, double beta, uint8_t goAgainsNum){
	Side toMove = s;
	size_t nMoves;
	auto* moves = b.validMoves(toMove, nMoves);
	if(nMoves == 0){
		if(b.stonesInWell(yourSide) > b.stonesInWell((Side)((int)yourSide ^ 1)))
			return std::make_pair(0, 1.0/0.0);
		else if(b.stonesInWell(yourSide) < b.stonesInWell((Side)((int)yourSide ^ 1)))
			return std::make_pair(0, -1.0/0.0);
		else
			return std::make_pair(0, 0);
	}
	if(depth == 0){
		/*
		int stoneNumDiff = b.stonesInWell(yourSide) - b.stonesInWell((Side)((int)yourSide ^ 1));
		int scoreDiff = 0;
		int stonesInFrontEmptyHoles = 0;
		uint8_t overflow = 0;

		for(int i = 0; i < 7; i++){
			stoneNumDiff -= b.stonesInHole((Side)((int)yourSide ^ 1), i);
			stoneNumDiff += b.stonesInHole(yourSide, i);
			if(b.stonesInHole(yourSide, i) == 0){
				stonesInFrontEmptyHoles += b.stonesInHole((Side)((int)yourSide ^ 1), i);
			}
			if(b.stonesInHole((Side)((int)yourSide ^ 1), i) == 0){
				stonesInFrontEmptyHoles -= b.stonesInHole(yourSide, i);
			}

			uint8_t stonesLeft = b.stonesInHole(yourSide, i);
			uint8_t currentHole = i;
			while(stonesLeft > 0){
				stonesLeft -= (7-currentHole);
				if(stonesLeft > 6){
					overflow += 6;
					stonesLeft -= 6;
					currentHole = 0;
				} else if(stonesLeft > 0){
					overflow += stonesLeft;
					stonesLeft = 0;
				}
			}
		}
		*/
		return std::make_pair(0, parameters[0]*scoreDiffHeuristic(yourSide, b) + parameters[1]*stoneNumDiff(yourSide, b) + parameters[2]*stonesInFrontEmptyHolesHeuristic(yourSide, b) - parameters[3]*overflow(yourSide, b));
	}
	// MAXIMIZE
	if(toMove == yourSide){
		std::pair<uint8_t, double> bestMove = std::make_pair(0, -1.0/0.0);
		// switch, maximum
		if(movesSoFar == 1){
			Board bCopy = b;
			yourSide = (Side)((int)yourSide ^ 1);
			std::pair<uint8_t, double> currentMove = minimax_alphabeta(depth-1, toMove, yourSide, bCopy, movesSoFar+1, alpha, beta, goAgainsNum);
			if(currentMove.second >= bestMove.second){
				bestMove.first = 7;
				bestMove.second = currentMove.second;
				alpha = bestMove.second;
			}
			yourSide = (Side)((int)yourSide ^ 1);
		}

		for(uint8_t i = 0; i < nMoves; i++) {
			Board bCopy = b;
			bool goAgain = bCopy.makeMove(toMove, moves[i]);
			if(!goAgain){
				toMove = (Side)((int)toMove ^ 1);
			} else{
				goAgainsNum++;
			}
			std::pair<uint8_t, double> currentMove = minimax_alphabeta(depth-1, toMove, yourSide, bCopy, movesSoFar+1, alpha, beta, goAgainsNum);
			if(currentMove.second >= bestMove.second){
				bestMove.first = i;
				bestMove.second = currentMove.second;
				alpha = bestMove.second;
				if(beta <= alpha){
					break;
				}
			}
		}
		return bestMove;
	// MINIMIZE
	} else {
		std::pair<uint8_t, double> bestMove = std::make_pair(0, 1.0/0.0); // SHOULD BE ADJUSTED TO MAX HEURISTIC
		// switch, minimum
		if(movesSoFar == 1){
			Board bCopy = b;
			yourSide = (Side)((int)yourSide ^ 1);
			std::pair<uint8_t, double> currentMove = minimax_alphabeta(depth-1, toMove, yourSide, bCopy, movesSoFar+1, alpha, beta, goAgainsNum);
			if(currentMove.second <= bestMove.second){
				bestMove.first = 7;
				bestMove.second = currentMove.second;
				beta = bestMove.second;
			}
			yourSide = (Side)((int)yourSide ^ 1);
		}
		for(uint8_t i = 0; i < nMoves; i++) {
			Board bCopy = b;
			bool goAgain = bCopy.makeMove(toMove, moves[i]);
			if(!goAgain){
				toMove = (Side)((int)toMove ^ 1);
			} else{
				goAgainsNum--;
			}
			std::pair<uint8_t, double> currentMove = minimax_alphabeta(depth-1, toMove, yourSide, bCopy, movesSoFar+1, alpha, beta, goAgainsNum);
			if(currentMove.second <= bestMove.second){
				bestMove.first = i;
				bestMove.second = currentMove.second;
				beta = bestMove.second;
				if(beta <= alpha){
					break;
				}
			}
		}
		return bestMove;
	}
}

static std::pair<uint8_t,double> minimax(uint8_t depth, Side s, Side yourSide, Board& b, size_t movesSoFar){
	Side toMove = s;
	size_t nMoves;
	auto* moves = b.validMoves(toMove, nMoves);

	if(depth == 0 || nMoves == 0)
		return std::make_pair(0, b.stonesInWell(yourSide) - b.stonesInWell((Side)((int)yourSide ^ 1)));
	// MAXIMIZE
	if(toMove == yourSide){
		std::pair<uint8_t, double> bestMove = std::make_pair(0, 0.0);
		// switch, maximum
		if(movesSoFar == 1){
			Board bCopy = b;
			yourSide = (Side)((int)yourSide ^ 1);
			std::pair<uint8_t, double> currentMove = minimax(depth-1, toMove, yourSide, bCopy, movesSoFar+1);
			if(currentMove.second >= bestMove.second){
				bestMove.first = 7;
				bestMove.second = currentMove.second;
				//std::cout << bestMove.second << std::endl;
			}
			yourSide = (Side)((int)yourSide ^ 1);
		}

		for(uint8_t i = 0; i < nMoves; i++) {
			Board bCopy = b;
			bool goAgain = bCopy.makeMove(toMove, moves[i]);
			if(!goAgain)
				toMove = (Side)((int)toMove ^ 1);
			std::pair<uint8_t, double> currentMove = minimax(depth-1, toMove, yourSide, bCopy, movesSoFar+1);
			if(currentMove.second >= bestMove.second){
				bestMove.first = i;
				bestMove.second = currentMove.second;
			}
		}
		return bestMove;
	// MINIMIZE
	} else {
		std::pair<uint8_t, double> bestMove = std::make_pair(0, 100.0); // SHOULD BE ADJUSTED TO MAX HEURISTIC
		// switch, minimum
		if(movesSoFar == 1){
			Board bCopy = b;
			yourSide = (Side)((int)yourSide ^ 1);
			std::pair<uint8_t, double> currentMove = minimax(depth-1, toMove, yourSide, bCopy, movesSoFar+1);
			if(currentMove.second <= bestMove.second){
				bestMove.first = 7;
				bestMove.second = currentMove.second;
			}
			yourSide = (Side)((int)yourSide ^ 1);
		}
		for(uint8_t i = 0; i < nMoves; i++) {
			Board bCopy = b;
			bool goAgain = bCopy.makeMove(toMove, moves[i]);
			if(!goAgain)
				toMove = (Side)((int)toMove ^ 1);
			std::pair<uint8_t, double> currentMove = minimax(depth-1, toMove, yourSide, bCopy, movesSoFar+1);
			if(currentMove.second <= bestMove.second){
				bestMove.first = i;
				bestMove.second = currentMove.second;
			}
		}
		return bestMove;
	}
}

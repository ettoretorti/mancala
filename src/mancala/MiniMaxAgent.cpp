#include "MiniMaxAgent.hpp"
#include <utility>
#include <iostream>
#include <cassert>

/**
*
*	index: The index of the best move. If the index == 7, then its a switch
*/
static std::pair<uint8_t,double> minimax(uint8_t depth, Side s, Side yourSide, Board& b, size_t movesSoFar);
static double someHeuristic(Board& b, Side s);

uint8_t MiniMaxAgent::makeMove(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	size_t nMoves;
	auto* moves = b.validMoves(s, nMoves);
	assert(nMoves > 0);

	uint8_t depth = 5; // SOME DEPTH
	Board bCopy = b;

	std::pair<uint8_t,double> result = minimax(depth, s, s, bCopy, movesSoFar);
	return moves[result.first];
}

static double someHeuristic(Board& b, Side s){
	return b.totalStones(s) - b.totalStones((Side)((int)s ^ 1));
}

static std::pair<uint8_t,double> minimax(uint8_t depth, Side s, Side yourSide, Board& b, size_t movesSoFar){
	if(depth == 0)
		return std::make_pair(0, someHeuristic(b, yourSide));
	Side toMove = s;
	size_t nMoves;

	// MAXIMIZE
	if(toMove == yourSide){
		auto* moves = b.validMoves(toMove, nMoves);
		std::pair<uint8_t, double> bestMove = std::make_pair(0, 0.0);
		// switch, maximum
		if(movesSoFar == 1){
			Board bCopy = b;
			yourSide = (Side)((int)yourSide ^ 1);
			std::pair<uint8_t, double> currentMove = minimax(depth-1, toMove, yourSide, bCopy, movesSoFar+1);
			if(currentMove.second >= bestMove.second){
				bestMove.first = 7;
				bestMove.second = currentMove.second;
				std::cout << bestMove.second << std::endl;
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
		auto* moves = b.validMoves(s, nMoves);
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

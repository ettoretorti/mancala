#include "MiniMaxAgent.hpp"

#include <cassert>

static uint8_t minimax(uint8_t depth, Side s, const Board& b, uint8_t nMoves);
uint8_t MiniMaxAgent::makeMove(const Board& b, Side s, bool canSwitch) {
	size_t nMoves;
	auto* moves = b.validMoves(s, nMoves);
	assert(nMoves > 0);

	uint8_t depth = 5; // SOME DEPTH
	return moves[minimax(depth, s, b, nMoves)];
}

static uint8_t minimax(uint8_t depth, Side s, const Board& b, size_t& nMoves){
	// TODO: ACCOUNT FOR SWITCHES
	Side& toMove = s;
	if(s == SOUTH){
		auto* moves = b.validMoves(s, nMoves);
		uint8_t maxScore = 0;
		uint8_t bestMoveIndex = 0;
		for(uint8_t i = 0; i < nMoves; i++) {
			Board bCopy = b;
			bool goAgain = bCopy.makeMove(s, moves[i]);
			if(!goAgain)
				toMove = (Side)((int)s ^ 1);
			uint8_t bestMove = minimax(depth, toMove, bCopy, nMoves);
			if(bestMove > maxScore){
				maxScore = bestMove;
				bestMoveIndex = i;
			}
		}
		if(depth == 0)
			return bestMoveIndex;
		return maxScore;
	} else{
		auto* moves = b.validMoves(s, nMoves);
		uint8_t minScore = 49; // or some max
		uint8_t bestMoveIndex = 0;
		for(uint8_t i = 0; i < nMoves; i++) {
			Board bCopy = b;
			bool goAgain = bCopy.makeMove(s, moves[i]);
			if(!goAgain)
				toMove = (Side)((int)s ^ 1);
			uint8_t bestMove = minimax(depth-1, toMove, bCopy, nMoves);
			if(bestMove < minScore){
				minScore = bestMove;
				bestMoveIndex = i;
			}
		}
		if(depth == 0)
			return bestMoveIndex;
		return minScore;
	}
}

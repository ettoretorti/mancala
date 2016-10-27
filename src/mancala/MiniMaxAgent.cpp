#include "MiniMaxAgent.hpp"

uint8_t MiniMaxAgent::makeMove(const Board& b, Side s, bool canSwitch) {
	size_t nMoves;
	auto* moves = b.validMoves(s, nMoves);
	if(nMoves == 0){
		 if(b.totalStones() == 0)
			return 1;
		 else
			return -1;
	}
	uint8_t depth = 5; // SOME DEPTH
	return moves[minimax(depth, depth, s, b, moves)];
}

uint8_t MiniMaxAgent::minimax(uint8_t depth, uint8_t curDepth, Side s, const Board& b, auto* moves){
	// TODO: ACCOUNT FOR SWITCHES
	Side& toMove = s;
	uint8_t prevScore = b.stonesInWell(s);

	if(s == SOUTH){
		uint8_t maxScore = 0;
		uint8_t bestMoveIndex = 0;
		for(uint8_t i = 0; i < nMoves; i++) {
			uint8_t stones = b.stonesInHole(s, moves[i]);
			bool goAgain = b.makeMove(s, moves[i]);
			if(!goAgain)
				toMove = (Side)((int)s ^ 1);
			uint8_t bestMove = minimax(depth, curDepth-1, toMove, b);
			if(bestMove > maxScore){
				maxScore = bestMove;
				bestMoveIndex = i;
			}
			b.undoMove(s, moves[i], prevScore, stones);
		}
		if(depth == curDepth)
			return bestMoveIndex;
		return maxScore;
	} else{
		uint8_t minScore = 49; // or some max
		uint8_t bestMoveIndex = 0;
		for(uint8_t i = 0; i < nMoves; i++) {
			uint8_t stones = b.stonesInHole(s, moves[i]);
			bool goAgain = b.makeMove(s, moves[i]);
			if(!goAgain)
				toMove = (Side)((int)s ^ 1);
			uint8_t bestMove = minimax(depth, curDepth-1, toMove, b);
			if(bestMove < minScore){
				minScore = bestMove;
				bestMoveIndex = i;
			}
			b.undoMove(s, moves[i], prevScore, stones);
		}
		if(depth == curDepth)
			return bestMoveIndex;
		return minScore;
	}
}
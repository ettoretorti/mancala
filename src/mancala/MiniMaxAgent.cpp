#include "MiniMaxAgent.hpp"
#include <utility>

#include <cassert>

static std::pair<uint8_t,double> minimax(uint8_t depth, uint8_t index, Side s, const Board& b);
static uint8_t someHeuristic(const Board& b);

uint8_t MiniMaxAgent::makeMove(const Board& b, Side s, bool canSwitch) {
	size_t nMoves;
	auto* moves = b.validMoves(s, nMoves);
	assert(nMoves > 0);

	uint8_t depth = 5; // SOME DEPTH
	std::pair<uint8_t,double> result = minimax(depth,0, s, b);
	return moves[result.first];
}

static uint8_t someHeuristic(const Board& b){
	return 0;
}

static std::pair<uint8_t,double> minimax(uint8_t depth, uint8_t index, Side s, const Board& b){
	// TODO: ACCOUNT FOR SWITCHES
	if(depth == 0)
		return std::make_pair(index, someHeuristic(b));
	Side& toMove = s;
	size_t nMoves;

	if(s == SOUTH){
		auto* moves = b.validMoves(s, nMoves);
		std::pair<uint8_t, double> bestMove = std::make_pair(0, 0.0);
		for(uint8_t i = 0; i < nMoves; i++) {
			Board bCopy = b;
			bool goAgain = bCopy.makeMove(s, moves[i]);
			if(!goAgain)
				toMove = (Side)((int)s ^ 1);
			std::pair<uint8_t, double> currentMove = minimax(depth-1, i, toMove, bCopy);

			if(currentMove.second >= bestMove.second)
				bestMove = currentMove;
		}
		return bestMove;
	} else{
		auto* moves = b.validMoves(s, nMoves);
		std::pair<uint8_t, double> bestMove = std::make_pair(0, 100.0); // SHOULD BE ADJUSTED TO MAX HEURISTIC
		for(uint8_t i = 0; i < nMoves; i++) {
			Board bCopy = b;
			bool goAgain = bCopy.makeMove(s, moves[i]);
			if(!goAgain)
				toMove = (Side)((int)s ^ 1);
			std::pair<uint8_t, double> currentMove = minimax(depth-1, i, toMove, bCopy);
			if(currentMove.second >= bestMove.second)
				bestMove = currentMove;
		}
		return bestMove;
	}
}

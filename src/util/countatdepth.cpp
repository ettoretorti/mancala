#include <mancala/Board.hpp>

#include <iostream>

uint64_t positions(size_t depth, Side whosTurn, const Board& b) {
	if(depth == 0) return 1;

	size_t nMoves;
	const auto* moves = b.validMoves(whosTurn, nMoves);
	
	uint64_t acc = 0;
	for(size_t i = 0; i < nMoves; i++) {
		Board tmp = b;
		bool again = tmp.makeMove(whosTurn, moves[i]);

		acc += positions(depth - 1, again ? whosTurn : (Side)(((int)whosTurn)^1), tmp);
	}

	return acc;
}

int main() {
	Board b;
	b.reset();
	
	size_t depth = 6;
	std::cout << positions(depth, SOUTH, b) << " leaf nodes at depth " << depth << std::endl;

	return 0;
}

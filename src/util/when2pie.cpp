#include <mancala/Board.hpp>
#include <mancala/Game.hpp>
#include <mancala/RandomAgent.hpp>

#include <iostream>

constexpr uint64_t GAMES_PER_MOVE = 1000000ull;

int main() {
	Board boards[7];

	for(uint8_t i = 0; i < 7; i++) {
		boards[i].reset();
		boards[i].makeMove(SOUTH, i);
	}

	uint64_t southWins[7] = { 0 };
	uint64_t northWins[7] = { 0 };

	//simulate positions
	#pragma omp parallel for
	for(size_t i = 0; i < 7; i++) {
		Game g(new RandomAgent(), new RandomAgent());

		for(size_t j = 0; j < GAMES_PER_MOVE; j++) {
			g.board() = boards[i];
			g.movesPlayed() = 3; // no more additional switching
			g.toMove() = NORTH;

			g.playAll();
			int diff = g.scoreDifference();

			if(diff > 0) southWins[i]++;
			if(diff < 0) northWins[i]++;
		}
	}

	for(size_t i = 0; i < 7; i++) {
		std::cout << "When first move is " << i << " "
		          << "SOUTH=" << southWins[i] << " NORTH=" << northWins[i]
		          << " DIFF=" << (((int64_t)southWins[i] - (int64_t)northWins[i])/(long double)GAMES_PER_MOVE)
		          << std::endl;
	}
}

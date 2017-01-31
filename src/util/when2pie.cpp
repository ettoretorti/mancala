#include <mancala/Board.hpp>
#include <mancala/Game.hpp>
#include <mancala/MCAgent.hpp>

#include <iostream>

constexpr uint64_t GAMES_PER_CHUNK = 1ull;
constexpr uint64_t CHUNKS_PER_MOVE = 4ull;

static thread_local Game g(new MCAgent(), new MCAgent());

int main() {
	Board boards[7];

	for(uint8_t i = 0; i < 7; i++) {
		boards[i].reset();
		boards[i].makeMove(SOUTH, i);
	}

	uint64_t southWins[7] = { 0 };
	uint64_t northWins[7] = { 0 };

	//simulate positions
	for(size_t i = 0; i < 7; i++) {
		uint64_t sWins = 0;
		uint64_t nWins = 0;

		#pragma omp parallel for schedule(dynamic) reduction(+:sWins,nWins)
		for(size_t j = 0; j < CHUNKS_PER_MOVE; j++) {

			for(size_t k = 0; k < GAMES_PER_CHUNK; k++) {
				g.board() = boards[i];
				g.movesPlayed() = 3; // no more additional switching
				g.toMove() = NORTH;

				while(g.board().stonesInWell(SOUTH) <= 49 && g.board().stonesInWell(NORTH) <= 49 && !g.isOver()) g.stepTurn();
				
				if(g.board().stonesInWell(SOUTH) > 49) {
					sWins += 2;
				} else if(g.board().stonesInWell(NORTH) > 49) {
					nWins += 2;
				} else {
					uint8_t scores[2] = { g.board().stonesInWell(SOUTH), g.board().stonesInWell(NORTH) };
					scores[int(g.toMove())^1] += 98 - scores[0] - scores[1];

					int diff = int(scores[0]) - scores[1];
					if(diff > 0) sWins += 2;
					if(diff < 0) nWins += 2;
					if(diff == 0) {
						nWins += 1; sWins += 1;
					}
				}
			}
		}

		southWins[i] = sWins;
		northWins[i] = nWins;
		std::cout << "DONE WITH POSITION " << i << std::endl;
	}

	for(size_t i = 0; i < 7; i++) {
		std::cout << "When first move is " << i << " "
		          << "SOUTH=" << southWins[i]/2.0 << " NORTH=" << northWins[i]/2.0
		          << " DIFF=" << (((double)southWins[i] - (double)northWins[i])/(long double)(2 * GAMES_PER_CHUNK * CHUNKS_PER_MOVE))
		          << std::endl;
	}
}

#include "FinalAgent.hpp"
#include "RandomAgent.hpp"
#include "MCAgent.hpp"

static thread_local MCAgent mc;
static thread_local RandomAgent ra;

uint8_t FinalAgent::makeMove(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	// Random playing on guaranteed win or loss
	uint8_t southScore = b.stonesInWell(SOUTH);
	uint8_t northScore = b.stonesInWell(NORTH);

	// Game is decided, no point in thinking
	if(southScore > 49 || northScore > 49) {
		return ra.makeMove(b, s, movesSoFar, lastMove);
	}

	// TODO: Check opening moves

	return mc.makeMove(b, s, movesSoFar, lastMove);
}

#include <mancala/input.hpp>
#include <mancala/output.hpp>
#include <mancala/Board.hpp>
#include <mancala/RandomAgent.hpp>
#include <mancala/MCAgent.hpp>

#include <memory>
#include <iostream>

int main() {
	using namespace std;
	using namespace input;

	auto agent = std::unique_ptr<MCAgent>(new MCAgent());
	//agent->useIterations() = false;
	//agent->timePerMove() = 2.0;

	auto msg = parseNext(cin);

	Start* startMsg = dynamic_cast<Start*>(msg.get());

	if(!startMsg) {
		cerr << "Did not get start message!" << endl;
		return -1;
	}

	Side curSide = SOUTH;
	Side ourSide = startMsg->side;
	Side oppSide = (Side)((int)ourSide^1);
	size_t movesPlayed = 0;
	uint8_t lastMove = 0;
	Board b;
	b.reset();

	while(true) {
		if(curSide == ourSide) {
			uint8_t move = agent->makeMove(b, ourSide, movesPlayed, lastMove);
			cout << output::move(move);
			cout.flush();

			if(move >= 7) {
				assert(movesPlayed == 1);

				std::swap(ourSide, oppSide);
				movesPlayed++;
				continue;
			}
		}

		msg = parseNext(cin);

		Change* chng = dynamic_cast<Change*>(msg.get());

		if(!chng) {
			cerr << "Did not get change message!" << endl;
			return -1;
		}

		movesPlayed++;
		lastMove = chng->lastMove;

		if(lastMove >= 7) {
			std::swap(ourSide, oppSide);
		} else {
			b.makeMove(curSide, lastMove);
		}

		if(!(b == chng->current)) {
			cerr << "Boards don't match!" << endl;
			return - 1;
		}

		curSide = chng->ourTurn ? ourSide : oppSide;
	}
}

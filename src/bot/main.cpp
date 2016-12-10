#include <mancala/input.hpp>
#include <mancala/output.hpp>
#include <mancala/Board.hpp>
#include <mancala/RandomAgent.hpp>

#include <memory>
#include <iostream>

int main() {
	using namespace std;
	using namespace input;

	auto agent = std::unique_ptr<Agent>(new RandomAgent());

	auto msg = parseNext(cin);

	Start* startMsg = dynamic_cast<Start*>(msg.get());

	if(!startMsg) {
		cerr << "Did not get start message!" << endl;
		return -1;
	}

	Side curSide = SOUTH;
	Side ourSide = startMsg->side;
	Side oppSide = (Side)((int)curSide^1);
	size_t movesPlayed =0 ;
	Board b;
	b.reset();

	while(true) {
		if(curSide == ourSide) {
			uint8_t move = agent->makeMove(b, ourSide, movesPlayed, 0);
			cout << output::move(move);

			if(move >= 7) std::swap(curSide, oppSide);
		}

		msg = parseNext(cin);


	}
}

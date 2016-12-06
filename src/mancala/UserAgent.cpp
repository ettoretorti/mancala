#include "UserAgent.hpp"

#include <iostream>

uint8_t UserAgent::makeMove(const Board& b, Side s, size_t movesSoFar, uint8_t lastMove) {
	using std::cout;
	using std::cin;

	cout << "Make a move for " << (s == NORTH ? "NORTH" : "SOUTH") << '\n';
	if(movesSoFar == 1) cout << "You are allowed to switch sides (enter anything >= 7)\n";
	cout << "Opponent's last move was " << (int)lastMove << std::endl;

	cout << "The state of the board is:\n";
	cout << "     6  5  4  3  2  1  0\n";
	cout << b.toString();
	cout << "     0  1  2  3  4  5  6\n";
	
	bool done = false;
	uint8_t move;
	
	while(!done) {
		cout << "Please enter your move: " << std::flush;

		cin >> move;
		if(cin.fail()) {
			cin.clear();
			std::string whatever;
			cin >> whatever;
		} else {
			done = true;
		}
	}
	move -= '0';
	cout << "Picked " << (int)move << std::endl;

	return move;
}

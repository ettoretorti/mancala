#include "UserAgent.hpp"

#include <iostream>

uint8_t UserAgent::makeMove(const Board& b, Side s, bool canSwitch) {
	using std::cout;
	using std::cin;

	cout << "Make a move for " << (s == NORTH ? "NORTH" : "SOUTH") << '\n';
	if(canSwitch) cout << "You are allowed to switch sides (enter anything >= 7)\n";

	cout << "The state of the board is:\n";
	cout << b.toString() << std::endl;
	
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
	cout << "Picked " << move << std::endl;

	return move;
}

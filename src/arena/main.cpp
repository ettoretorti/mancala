#include <mancala/Game.hpp>
#include <mancala/RandomAgent.hpp>

#include <iostream>

int main() {
	Agent* p1 = new RandomAgent();
	Agent* p2 = new RandomAgent();

	Game g(p1, p2);
	g.reset();

	g.playAll();

	std::cout << "Game finished in " << g.movesPlayed() << " moves.\n";
	std::cout << "The score difference is " << g.scoreDifference() << std::endl;

	return 0;
}

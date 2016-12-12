#include <mancala/Game.hpp>
#include <mancala/RandomAgent.hpp>
#include <mancala/MCAgent.hpp>
#include <mancala/UserAgent.hpp>
#include <mancala/MiniMaxAgent.hpp>
#include <iostream>

int main() {
	//Agent* p1 = new MiniMaxAgent();
	//Agent* p1 = new UserAgent();
	Agent* p1 = new MiniMaxAgent();
	Agent* p2 = new MCAgent();

	bool printIntermediate = true;

	Game g(p1, p2);
	g.reset();

	g.takeTimings() = true;

	if(!printIntermediate) {
		g.playAll();
	} else {
		while(!g.isOver()) {
			std::cout << g.movesPlayed() << " moves played\n";
			std::cout << (g.toMove() == SOUTH ? "SOUTH" : "NORTH") << " to move\n";
			std::cout << "board state is:\n";
			std::cout << g.board().toString();
			g.stepTurn();
			std::cout << "made move " << (int)g.lastMove() << "\n" << std::endl;
		}
		std::cout << g.movesPlayed() << " moves played\n";
		std::cout << "game over\n";
		std::cout << "board state is:\n";
		std::cout << g.board().toString();
		std::cout << std::endl;
	}

	std::cout << "Game finished in " << g.movesPlayed() << " moves\n";
	std::cout << "The score difference is " << g.scoreDifference() << std::endl;
	std::cout << "Player 1 took " << g.p1Time() << " seconds\n";
	std::cout << "Player 2 took " << g.p2Time() << " seconds\n";

	return 0;
}

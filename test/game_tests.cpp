#include <gtest/gtest.h>

#include <mancala/Game.hpp>

void clearBoard(Board& b) {
	for(int i = 0; i < 7; i++) {
		b.stonesInHole(NORTH, i) = b.stonesInHole(SOUTH, i) =  0;
	}
	b.stonesInWell(NORTH) = b.stonesInWell(SOUTH) = 0;

	b.recalcMoves();
}

TEST(Game, TestGameOver) {
	Game g(nullptr, nullptr);
	Board& b = g.board();
	clearBoard(b);

	ASSERT_TRUE(g.isOver());
	
	b.stonesInHole(NORTH, 0) = 1;
	b.recalcMoves();
	ASSERT_TRUE(g.isOver());

	g.toMove() = NORTH;
	ASSERT_FALSE(g.isOver());

	b.stonesInHole(NORTH, 0) = 0;
	b.stonesInHole(SOUTH, 3) = 98;
	b.recalcMoves();

	g.toMove() = SOUTH;
	ASSERT_FALSE(g.isOver());
}

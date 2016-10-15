#include <gtest/gtest.h>

#include <mancala/Game.hpp>

TEST(Game, TestGameOver) {
	Game g(nullptr, nullptr);
	Board& b = g.board();
	b.clear();

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

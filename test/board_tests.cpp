#include <gtest/gtest.h>

#include <cstring>
#include <functional>

#include <mancala/Board.hpp>

TEST(Board, Clear) {
	Board b;
	b.clear();
	
	for(size_t i = 0; i < 7; i++) {
		EXPECT_EQ(0, b.stonesInHole(NORTH, i));
		EXPECT_EQ(0, b.stonesInHole(SOUTH, i));
	}

	EXPECT_EQ(0, b.stonesInWell(NORTH));
	EXPECT_EQ(0, b.stonesInWell(SOUTH));

	size_t nMoves;
	b.validMoves(NORTH, nMoves);
	EXPECT_EQ(0u, nMoves);

	b.validMoves(SOUTH, nMoves);
	EXPECT_EQ(0u, nMoves);
}

TEST(Board, Reset) {
	Board b;
	b.reset();

	for(size_t i = 0; i < 7; i++) {
		EXPECT_EQ(7, b.stonesInHole(NORTH, i));
		EXPECT_EQ(7, b.stonesInHole(SOUTH, i));
	}

	EXPECT_EQ(0, b.stonesInWell(NORTH));
	EXPECT_EQ(0, b.stonesInWell(SOUTH));


	size_t nMoves;
	bool movesCovered[7];

	memset(movesCovered, 0, 7);
	const uint8_t* moves = b.validMoves(NORTH, nMoves);

	EXPECT_EQ(7u, nMoves);
	for(size_t i = 0; i < nMoves; i++) {
		EXPECT_LT(moves[i], 7);
		movesCovered[moves[i]] = true;
	}

	for(size_t i = 0; i < 7; i++) {
		EXPECT_TRUE(movesCovered[i]);
	}
	

	memset(movesCovered, 0, 7);
	moves = b.validMoves(SOUTH, nMoves);


	EXPECT_EQ(7u, nMoves);
	for(size_t i = 0; i < nMoves; i++) {
		EXPECT_LT(moves[i], 7);
		movesCovered[moves[i]] = true;
	}

	for(size_t i = 0; i < 7; i++) {
		EXPECT_TRUE(movesCovered[i]);
	}
}

TEST(Board, ValidMoves) {
	Board b;
	auto nHole = [&](size_t i) -> uint8_t& { return b.stonesInHole(NORTH, i); };
	auto sHole = [&](size_t i) -> uint8_t& { return b.stonesInHole(SOUTH, i); };

	b.clear();

	nHole(0) = 1;
	b.recalcMoves();

	size_t nMoves;
	auto* moves = b.validMoves(NORTH, nMoves);

	EXPECT_EQ(1u, nMoves);
	EXPECT_EQ(0u, moves[0]);

	moves = b.validMoves(SOUTH, nMoves);
	EXPECT_EQ(0u, nMoves);

	
	b.clear();
	sHole(0) = 1;
	b.recalcMoves();

	moves = b.validMoves(SOUTH, nMoves);

	EXPECT_EQ(1u, nMoves);
	EXPECT_EQ(0u, moves[0]);

	moves = b.validMoves(NORTH, nMoves);
	EXPECT_EQ(0u, nMoves);

	
	b.clear();
	nHole(0) = nHole(2) = nHole(4) = nHole(6) = 1;
	sHole(1) = sHole(3) = sHole(5) = 1;
	b.recalcMoves();

	moves = b.validMoves(NORTH, nMoves);
	EXPECT_EQ(4u, nMoves);

	bool movesCovered[7];
	memset(movesCovered, 0, 7);

	for(size_t i = 0; i < nMoves; i++) {
		EXPECT_LT(moves[i], 7);
		movesCovered[moves[i]] = true;
	}

	EXPECT_TRUE(movesCovered[0]); EXPECT_TRUE(movesCovered[2]);
	EXPECT_TRUE(movesCovered[4]); EXPECT_TRUE(movesCovered[6]);

	EXPECT_FALSE(movesCovered[1]); EXPECT_FALSE(movesCovered[3]);
	EXPECT_FALSE(movesCovered[5]);

	moves = b.validMoves(SOUTH, nMoves);
	EXPECT_EQ(3u, nMoves);

	memset(movesCovered, 0, 7);

	for(size_t i = 0; i < nMoves; i++) {
		EXPECT_LT(moves[i], 7);
		movesCovered[moves[i]] = true;
	}
	
	EXPECT_TRUE(movesCovered[1]); EXPECT_TRUE(movesCovered[3]);
	EXPECT_TRUE(movesCovered[5]);

	EXPECT_FALSE(movesCovered[0]); EXPECT_FALSE(movesCovered[2]);
	EXPECT_FALSE(movesCovered[4]); EXPECT_FALSE(movesCovered[6]);
}

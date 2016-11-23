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

	b.validMoves(SOUTH, nMoves);
	EXPECT_EQ(0u, nMoves);

	
	b.clear();
	sHole(0) = 1;
	b.recalcMoves();

	moves = b.validMoves(SOUTH, nMoves);

	EXPECT_EQ(1u, nMoves);
	EXPECT_EQ(0u, moves[0]);

	b.validMoves(NORTH, nMoves);
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

TEST(Board, SmallMove) {
	Board b;
	b.clear();

	auto nHole = [&](size_t i) -> uint8_t& { return b.stonesInHole(NORTH, i); };
	auto sHole = [&](size_t i) -> uint8_t& { return b.stonesInHole(SOUTH, i); };

	sHole(6) = 2;
	b.recalcMoves();

	bool goAgain = b.makeMove(SOUTH, 6);

	EXPECT_FALSE(goAgain);
	EXPECT_EQ(1u, b.stonesInWell(SOUTH));
	EXPECT_EQ(0u, b.stonesInWell(NORTH));
	EXPECT_EQ(1u, nHole(0));

	for(size_t i = 0; i < 7; i++) {
		EXPECT_EQ(0u, sHole(i));
	}

	for(size_t i = 1; i < 7; i++) {
		EXPECT_EQ(0u, nHole(i));
	}

	size_t nMoves;
	b.validMoves(SOUTH, nMoves);

	EXPECT_EQ(0u, nMoves);

	const uint8_t* moves = b.validMoves(NORTH, nMoves);
	EXPECT_EQ(1u, nMoves);

	EXPECT_EQ(0u, moves[0]);
}

TEST(Board, LastStoneInWell) {
	Board b;
	b.clear();

	auto nHole = [&](size_t i) -> uint8_t& { return b.stonesInHole(NORTH, i); };
	auto sHole = [&](size_t i) -> uint8_t& { return b.stonesInHole(SOUTH, i); };

	nHole(0) = 7;
	b.recalcMoves();

	bool goAgain = b.makeMove(NORTH, 0);

	EXPECT_TRUE(goAgain);
	EXPECT_EQ(1u, b.stonesInWell(NORTH));
	EXPECT_EQ(0u, b.stonesInWell(SOUTH));

	EXPECT_EQ(0u, nHole(0));

	for(size_t i = 0; i < 7; i++) {
		EXPECT_EQ(0u, sHole(i));
	}

	for(size_t i = 1; i < 7; i++) {
		EXPECT_EQ(1u, nHole(i));
	}

	size_t nMoves;
	b.validMoves(SOUTH, nMoves);

	EXPECT_EQ(0u, nMoves);

	const uint8_t* moves = b.validMoves(NORTH, nMoves);
	EXPECT_EQ(6u, nMoves);

	bool movesAvail[7] = { 0 };

	for(size_t i = 0; i < nMoves; i++) {
		movesAvail[moves[i]] = true;
	}

	EXPECT_FALSE(movesAvail[0]);
	for(size_t i = 1; i < 7; i++) {
		EXPECT_TRUE(movesAvail[i]);
	}
}

TEST(Board, Capture) {
	Board b;
	b.clear();

	auto nHole = [&](size_t i) -> uint8_t& { return b.stonesInHole(NORTH, i); };
	auto sHole = [&](size_t i) -> uint8_t& { return b.stonesInHole(SOUTH, i); };

	sHole(0) = 15;
	nHole(6) = 69;
	b.recalcMoves();

	bool goAgain = b.makeMove(SOUTH, 0);

	EXPECT_FALSE(goAgain);
	EXPECT_EQ(72u, b.stonesInWell(SOUTH));
	EXPECT_EQ(0u, b.stonesInWell(NORTH));

	EXPECT_EQ(0u, sHole(0));
	for(size_t i = 1; i < 7; i++) {
		EXPECT_EQ(1u, sHole(i));
	}

	for(size_t i = 0; i < 6; i++) {
		EXPECT_EQ(1u, nHole(i));
	}
	EXPECT_EQ(0u, nHole(6));

	size_t nMoves;
	const uint8_t* moves = b.validMoves(SOUTH, nMoves);
	EXPECT_EQ(6u, nMoves);

	bool movesAvail[7] = { 0 };
	for(size_t i = 0; i < nMoves; i++) {
		movesAvail[moves[i]] = true;
	}

	EXPECT_FALSE(movesAvail[0]);
	for(size_t i = 1; i < 7; i++) {
		EXPECT_TRUE(movesAvail[i]);
	}

	moves = b.validMoves(NORTH, nMoves);
	EXPECT_EQ(6u, nMoves);

	memset(movesAvail, 0, 7);
	for(size_t i = 0; i < nMoves; i++) {
		movesAvail[moves[i]] = true;
	}

	for(size_t i = 0; i < 6; i++) {
		EXPECT_TRUE(movesAvail[i]);
	}
	EXPECT_FALSE(movesAvail[6]);
}

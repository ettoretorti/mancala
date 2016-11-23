#include <gtest/gtest.h>

#include <mancala/input.hpp>

#include <sstream>

TEST(IO, GameOver) {
	std::stringstream s;
	s << "END\n";

	auto ptr = input::parseNext(s);

	ASSERT_TRUE(dynamic_cast<input::GameOver*>(ptr.get()));
}

TEST(IO, StartNorth) {
	std::stringstream s;
	s << "START;North\n";

	auto ptr = input::parseNext(s);
	input::Start* str = dynamic_cast<input::Start*>(ptr.get());

	ASSERT_TRUE(str);
	EXPECT_EQ(NORTH, str->side);
}

TEST(IO, StartSouth) {
	std::stringstream s;
	s << "START;South\n";

	auto ptr = input::parseNext(s);
	input::Start* str = dynamic_cast<input::Start*>(ptr.get());

	ASSERT_TRUE(str);
	EXPECT_EQ(SOUTH, str->side);
}

TEST(IO, Change) {
	std::stringstream s;
	s << "CHANGE;SWAP;"
          << "1,2,3,4,5,6,7,8,"
	  << "9,10,11,12,13,14,15,16;"
	  << "YOU\n";

	auto ptr = input::parseNext(s);

	input::Change* chng = dynamic_cast<input::Change*>(ptr.get());
	
	ASSERT_TRUE(chng);
	
	EXPECT_EQ(7, chng->lastMove);
	EXPECT_TRUE(chng->ourTurn);
	
	for(size_t i = 0; i < 7; i++) {
		EXPECT_EQ(i+1, chng->current.stonesInHole(NORTH, i));
		EXPECT_EQ(i+9, chng->current.stonesInHole(SOUTH, i));
	}

	EXPECT_EQ(8, chng->current.stonesInWell(NORTH));
	EXPECT_EQ(16, chng->current.stonesInWell(SOUTH));
}

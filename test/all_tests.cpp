#include <gtest/gtest.h>

#include "board_tests.cpp"
#include "game_tests.cpp"
#include "io_tests.cpp"

int main(int argc, char** argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

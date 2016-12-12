#pragma once

#include "Board.hpp"

#include <cstdint>
#include <istream>
#include <unordered_map>

namespace binutils {
	// little endian, or else!
	std::unordered_map<Board, uint8_t> readBook(std::istream& in);
	std::unordered_map<Board, float>  readVBook(std::istream& in);
}

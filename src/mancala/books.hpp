#pragma once

#include "Board.hpp"

#include <cstddef>
#include <utility>
#include <unordered_map>

namespace books {
	std::pair<const char*, size_t> southBookBin();
	std::pair<const char*, size_t> northBookBin();

	const std::unordered_map<Board, uint8_t>& southBook();
	const std::unordered_map<Board, uint8_t>& northBook();
}

#include "binutils.hpp"

namespace binutils {
	std::unordered_map<Board, uint8_t> readBook(std::istream& in) {
		std::unordered_map<Board, uint8_t> toRet;

		uint32_t size;
		in.read((char*)&size, sizeof(uint32_t));

		Board cur;
		cur.clear();

		for(size_t i = 0; i < size; i++) {
			in.read((char*)&cur, 16);
			cur.recalcMoves();
			uint8_t move;
			in.read((char*)&move, 1);

			toRet[cur] = move;
		}

		return toRet;
	}

	std::unordered_map<Board, float> readVBook(std::istream& in) {
		std::unordered_map<Board, float> toRet;

		uint32_t size;
		in.read((char*)&size, sizeof(uint32_t));

		Board cur;
		cur.clear();

		for(size_t i = 0; i < size; i++) {
			in.read((char*)&cur, 16);
			cur.recalcMoves();
			float val;
			in.read((char*)&val, sizeof(float));

			toRet[cur] = val;
		}

		return toRet;
	}
}

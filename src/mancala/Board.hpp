#pragma once

#include <cstdint>
#include <cassert>
#include <string>
#include <functional>

enum Side { SOUTH = 0, NORTH = 1 };

class Board {
public:
	Board(); //uninitialized by default to save time
	Board(const Board& o);
	Board(Board&& o);
	Board& operator=(const Board& o);
	Board& operator=(Board&& o);

	void clear();
	void reset();
	void recalcMoves();

	inline uint8_t stonesInHole(Side side, size_t holeNo) const;
	inline uint8_t stonesInWell(Side side) const;
	
	inline uint8_t& stonesInHole(Side side, size_t holeNo);
	inline uint8_t& stonesInWell(Side side);

	bool makeMove(Side side, size_t holeNo);

	inline const uint8_t* validMoves(Side side, size_t& nMoves) const;

	std::string toString() const;

private:
	//This totals out to 32 bytes (EXACTLY HALF A CACHE LINE)
	uint8_t nHoles_[7];
	uint8_t sHoles_[7];
	uint8_t sScore_;
	uint8_t nScore_;

	uint8_t nMoves_[7];
	uint8_t sMoves_[7];
	uint8_t noNMoves_;
	uint8_t noSMoves_;

	void addMove(Side side, size_t holeNo);
	void placeAStone(Side curSide, uint8_t curHole);
	void removeMove(Side side, size_t holeNo);
	void removeAStone(Side curSide, uint8_t curHole);
};

bool operator==(const Board& b1, const Board& b2);

inline uint8_t Board::stonesInHole(Side side, size_t holeNo) const {
	assert(side == SOUTH || side == NORTH);
	assert(holeNo < 7);

	return side == SOUTH ? sHoles_[holeNo] : nHoles_[holeNo];
}

inline uint8_t Board::stonesInWell(Side side) const {
	assert(side == SOUTH || side == NORTH);

	return side == SOUTH ? sScore_ : nScore_;
}

inline uint8_t& Board::stonesInHole(Side side, size_t holeNo) {
	assert(side == SOUTH || side == NORTH);
	assert(holeNo < 7);

	return side == SOUTH ? sHoles_[holeNo] : nHoles_[holeNo];
}

inline uint8_t& Board::stonesInWell(Side side) {
	assert(side == SOUTH || side == NORTH);

	return side == SOUTH ? sScore_ : nScore_;
}

inline const uint8_t* Board::validMoves(Side side, size_t& nMoves) const {
	nMoves = side == SOUTH ? noSMoves_ : noNMoves_;
	return side == SOUTH ? sMoves_ : nMoves_;
}

namespace std {

template<> struct hash<Board> {
	typedef Board argument_type;
	typedef std::size_t result_type;

	// Taken from boost
	inline void combine(std::size_t& hash, uint8_t val) const {
		std::hash<uint8_t> h;

		hash ^= h(val) + 0x9e3779b9 + (hash<<6) + (hash>>2);
	}

	result_type operator()(const argument_type& b) const {
		std::hash<uint8_t> byteHasher;
		result_type toRet = byteHasher(b.stonesInHole(NORTH, 0));

		for(size_t i = 1; i < 7; i++) {
			combine(toRet, b.stonesInHole(NORTH, i));
		}

		for(size_t i = 0; i < 7; i++) {
			combine(toRet, b.stonesInHole(SOUTH, i));
		}

		combine(toRet, b.stonesInWell(NORTH));
		combine(toRet, b.stonesInWell(SOUTH));

		return toRet;
	}
};

}

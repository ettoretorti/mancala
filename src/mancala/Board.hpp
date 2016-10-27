#pragma once

#include <cstdint>
#include <string>

enum Side { SOUTH = 0, NORTH = 1 };

class alignas(32) Board {
public:
	Board(); //uninitialized by default to save time
	Board(const Board& o);
	Board(Board&& o);
	Board& operator=(const Board& o);
	Board& operator=(Board&& o);

	void clear();
	void reset();
	void recalcMoves();

	uint8_t totalStones(Side side) const;
	uint8_t stonesInHole(Side side, size_t holeNo) const;
	uint8_t stonesInWell(Side side) const;
	
	uint8_t& stonesInHole(Side side, size_t holeNo);
	uint8_t& stonesInWell(Side side);

	bool makeMove(Side side, size_t holeNo);
	bool undoMove(Side side, size_t holeNo, uint8_t prevScore, uint8_t stones);

	const uint8_t* validMoves(Side side, size_t& nMoves) const;

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
	
	uint8_t totalNStones_;
	uint8_t totalSStones_;

	void addMove(Side side, size_t holeNo);
	void placeAStone(Side curSide, uint8_t curHole);
	void removeMove(Side side, size_t holeNo);
	void removeAStone(Side curSide, uint8_t curHole);
};

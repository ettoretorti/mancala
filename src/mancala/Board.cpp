#include "Board.hpp"

#include <cstring>
#include <cassert>
#include <string>
#include <sstream>


//static utils
static Side opposite(Side s) {
	return (Side)((int)s ^ 1);
}

Board::Board() {}

Board::Board(const Board& o) {
	memmove(this, &o, sizeof(Board));
}

Board::Board(Board&& o) {
	memmove(this, &o, sizeof(Board));
}

Board& Board::operator=(const Board& o) {
	memmove(this, &o, sizeof(Board));
	return *this;
}

Board& Board::operator=(Board&& o) {
	memmove(this, &o, sizeof(Board));
	return *this;
}

void Board::reset() {
	for(uint8_t i = 0; i < 7; i++) {
		nHoles_[i] = sHoles_[i] = 7;
		nMoves_[i] = sMoves_[i] = i;
	}
	nScore_ = sScore_ = 0;
	noNMoves_ = noSMoves_ = 7;
}

void Board::recalcMoves() {
	noNMoves_ = noSMoves_ = 0;
	for(uint8_t i = 0; i < 7; i++) {
		if(nHoles_[i]) {
			nMoves_[noNMoves_++] = i;
		}
		
		if(sHoles_[i]) {
			sMoves_[noSMoves_++] = i;
		}
	}
}

uint8_t Board::stonesInHole(Side side, size_t holeNo) const {
	assert(side == SOUTH || side == NORTH);
	assert(holeNo < 7);

	return side == SOUTH ? sHoles_[holeNo] : nHoles_[holeNo];
}

uint8_t Board::stonesInWell(Side side) const {
	assert(side == SOUTH || side == NORTH);
	
	return side == SOUTH ? sScore_ : nScore_;
}

uint8_t& Board::stonesInHole(Side side, size_t holeNo) {
	assert(side == SOUTH || side == NORTH);
	assert(holeNo < 7);

	return side == SOUTH ? sHoles_[holeNo] : nHoles_[holeNo];
}

uint8_t& Board::stonesInWell(Side side) {
	assert(side == SOUTH || side == NORTH);
	
	return side == SOUTH ? sScore_ : nScore_;
}

void Board::addMove(Side side, size_t holeNo) {
	assert(side == SOUTH || side == NORTH);
	assert(holeNo < 7);

	uint8_t* arr    = side == SOUTH ? sMoves_ : nMoves_;
	uint8_t& nMoves = side == SOUTH ? noSMoves_ : noNMoves_;

	for(uint8_t i = 0; i < nMoves; i++) {
		if(arr[i] == holeNo) return;
	}

	arr[nMoves++] = holeNo;
}

void Board::removeMove(Side side, size_t holeNo) {
	assert(side == SOUTH || side == NORTH);
	assert(holeNo < 7);
	
	uint8_t* arr    = side == SOUTH ? sMoves_ : nMoves_;
	uint8_t& nMoves = side == SOUTH ? noSMoves_ : noNMoves_;

	for(uint8_t i = 0; i < nMoves; i++) {
		if(arr[i] == holeNo) {
			arr[i] = arr[nMoves-1];
			nMoves--;
			break;
		}
	}
}

void Board::makeMove(Side side, size_t holeNo) {
	assert(side == SOUTH || side == NORTH);
	assert(holeNo < 7);
	assert(side == SOUTH ? sHoles_[holeNo] : nHoles_[holeNo]);
	
	// ╔> 7 PLAYER HOLES ═> 7 OPPONENT HOLES ═> PLAYER WELL ╗
	// ╚════════════════════════════════════════════════════╝
	
	uint8_t stonesLeft = stonesInHole(side, holeNo);
	stonesInHole(side, holeNo) = 0;
	if(stonesLeft < 15) removeMove(side, holeNo);
	
	uint8_t prevHole = 7;
	Side    prevSide = SOUTH;
	uint8_t curHole  = holeNo == 6 ? 0 : holeNo + 1;
	Side    curSide  = curHole == 0 ? opposite(side) : side;

	//TODO this is terrible. rewrite with closed form
	while(stonesLeft > 0) {
		prevHole = curHole;
		prevSide = curSide;

		if(++stonesInHole(curSide, curHole) == 1) {
			addMove(curSide, curHole);	
		}

		if(++curHole > 6) {
			if(curSide == side) {
				curHole = 0;
				curSide = opposite(side);
			} else if(stonesLeft > 1) {
				if(side == SOUTH) {
					sScore_++;
				} else {
					nScore_++;
				}
				stonesLeft--;

				curSide = side;
				curHole = 0;
			}
		}

		stonesLeft--;
	}

	//check for capture
	if(prevHole != 7 && prevSide == side && stonesInHole(side, prevHole) == 1) {
		stonesInWell(side) += 1 + stonesInHole(opposite(side), 6 - prevHole);
		stonesInHole(side, prevHole) = 0;
		stonesInHole(opposite(side), 6 - prevHole) = 0;
		removeMove(side, prevHole);
		removeMove(opposite(side), 6 - prevHole);
	}
}

const uint8_t* Board::validMoves(Side side, size_t& nMoves) const {
	nMoves = side == SOUTH ? noSMoves_ : noNMoves_;
	return side == SOUTH ? sMoves_ : nMoves_;
}

std::string Board::toString() const {
	std::stringstream buf;

	buf << "NORTH SCORE: " << nScore_ << '\n';
	buf << "SOUTH SCORE: " << sScore_ << '\n';
	buf << '\n';

	for(size_t i = 0; i < 6; i++) {
		buf << nHoles_[6-i] << " ";
	}
	buf << nHoles_[0] << '\n';

	for(size_t i = 0; i < 5; i++) {
		buf << sHoles_[i] << " ";
	}
	buf << sHoles_[6] << '\n';

	return buf.str();
}

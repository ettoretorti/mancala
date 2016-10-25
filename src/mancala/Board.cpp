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

void Board::clear() {
	for(uint8_t i = 0; i < 7; i++) {
		nHoles_[i] = sHoles_[i] = 0;
	}
	nScore_ = sScore_ = 0;
	noNMoves_ = noSMoves_ = 0;
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

void Board::placeAStone(Side curSide, uint8_t curHole){
	++stonesInHole(curSide, curHole);
	if(stonesInHole(curSide, curHole) == 1) {
		addMove(curSide, curHole);	
	}
}

bool Board::makeMove(Side side, size_t holeNo) {	
	// ╔> 7 PLAYER HOLES ═> PLAYER WELL ═> 7 OPPONENT HOLES ╗
	// ╚════════════════════════════════════════════════════╝

	uint8_t stonesLeft = stonesInHole(side, holeNo);
	stonesInHole(side, holeNo) = 0;
	if(stonesLeft < 15)
		removeMove(side, holeNo);

	uint8_t curHole  = holeNo;
	Side    curSide  = side;
	bool goAgain = false;

	/*
		Points to the Well are added if:
			1. The player is on his side, and has reached the end
			2. The player adds his last piece in an empty hole
		In case 2, the player captures the coresponding pieces on his opponents side as well.
		
		Keep track of changes in valid moves.
		Add a stone to the current bucket if no point change
	*/
	if(stonesLeft > 15){
		uint8_t iterationsNum = stonesLeft % 15;
		uint8_t perHole = stonesLeft / 15;

		for(uint8_t i = 0; i < 7; i++) {
			stonesInHole(curSide, i) += perHole;
			stonesInHole(opposite(curSide), i) += perHole;
			nMoves_[i] = sMoves_[i] = i;
		}
		noSMoves_ = noNMoves_ = 7;
		stonesInWell(curSide) += perHole;
		stonesLeft = iterationsNum;
		// we cannot have empty capture as we already filled all holes
		while(stonesLeft > 0){
			if(curHole == 6){
				if(curSide == side){
					++stonesInWell(curSide);
					stonesLeft--;
				}
				curHole = 0;
				curSide = opposite(side);
				if(stonesLeft > 0){
					++stonesInHole(curSide, curHole);
					stonesLeft--;
				} else {
					goAgain = true;
				}
			} else {
				curHole++;
				++stonesInHole(curSide, curHole);
				stonesLeft--;
			}
		}
	} else {
		while(stonesLeft > 0){
			if(curHole == 6){
				if(curSide == side){
					++stonesInWell(curSide);
					stonesLeft--;
				}
				curHole = 0;
				curSide = opposite(side);
				if(stonesLeft > 0){
					placeAStone(curSide, curHole);
					stonesLeft--;
				} else {
					goAgain = true;
				}
			} else {
				curHole++;
				placeAStone(curSide, curHole);
				stonesLeft--;
			}
		}
		// Empty Hole Capture
		if(curSide == side && stonesInHole(curSide, curHole) == 1
				&& stonesInHole(opposite(curSide), 6 - curHole) > 0){
			stonesInWell(curSide) += stonesInHole(opposite(curSide), 6-curHole) + 1;
			removeMove(opposite(curSide), 6-curHole);
			removeMove(curSide, curHole);

			stonesInHole(opposite(curSide), 6-curHole) = 0;
			stonesInHole(curSide, curHole) = 0;
			goAgain = true;
		}
	}

	return goAgain;
}

const uint8_t* Board::validMoves(Side side, size_t& nMoves) const {
	nMoves = side == SOUTH ? noSMoves_ : noNMoves_;
	return side == SOUTH ? sMoves_ : nMoves_;
}

std::string Board::toString() const {
	std::stringstream buf;

	buf << "NORTH SCORE: " << (size_t)nScore_ << '\n';
	buf << "SOUTH SCORE: " << (size_t)sScore_ << '\n';
	buf << '\n';

	for(size_t i = 0; i < 6; i++) {
		buf << (size_t)nHoles_[6-i] << " ";
	}
	buf << (size_t)nHoles_[0] << '\n';

	for(size_t i = 0; i < 6; i++) {
		buf << (size_t)sHoles_[i] << " ";
	}
	buf << (size_t)sHoles_[6] << '\n';

	return buf.str();
}

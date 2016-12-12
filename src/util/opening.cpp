#include <mancala/Board.hpp>
#include <mancala/MCAgent.hpp>

#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <unordered_map>

namespace std {

template<> struct hash<std::pair<Board, Side>> {
	typedef std::pair<Board, Side> argument_type;
	typedef std::size_t result_type;

	result_type operator()(const argument_type& b) const {
		std::hash<Board> k;
		std::hash<int> kk;
		size_t h = k(b.first);
		h = kk(int(b.second)) + 0x9e3779b9 + (h<<6) + (h>>2);
		return h;
	}
};

}

void gen_positions_split(size_t depth, Side whosTurn, const Board& b, std::vector<Board>& sb, std::vector<Board>& nb) {
	if(depth == 0) {
		if(whosTurn == SOUTH) sb.push_back(b);
		else                  nb.push_back(b);

		return;
	}

	size_t nMoves;
	const auto* moves = b.validMoves(whosTurn, nMoves);
	bool firstMove = whosTurn == SOUTH && b.stonesInWell(NORTH) == 0;
	
	for(size_t i = 0; i < nMoves; i++) {
		Board tmp = b;
		bool again = tmp.makeMove(whosTurn, moves[i]);

		gen_positions_split(depth - 1, (again && !firstMove) ? whosTurn : (Side)(((int)whosTurn)^1), tmp, sb, nb);
	}
}

void gen_positions(size_t depth, Side whosTurn, const Board& b, std::vector<std::pair<Board, Side>>& acc) {
	if(depth == 0) {
		acc.push_back(std::make_pair(b, whosTurn));
		return;
	}

	size_t nMoves;
	const auto* moves = b.validMoves(whosTurn, nMoves);
	
	for(size_t i = 0; i < nMoves; i++) {
		Board tmp = b;
		bool again = tmp.makeMove(whosTurn, moves[i]);

		gen_positions(depth - 1, again ? whosTurn : (Side)(((int)whosTurn)^1), tmp, acc);
	}
}

std::pair<uint8_t, float> calcValue(const Board& pos, Side s, const std::unordered_map<std::pair<Board, Side>, std::pair<uint8_t, float>>& vv) {
	auto it = vv.find(std::make_pair(pos, s));
	if(it != vv.end()) {
		return it->second;
	}
	
	size_t nMoves;
	const auto* moves = pos.validMoves(s, nMoves);

	std::pair<uint8_t, float> bestVal = std::make_pair(moves[0], -1.0/0.0);

	for(size_t i = 0; i < nMoves; i++) {
		Board cpy = pos;
		bool ga = cpy.makeMove(s, moves[i]);

		auto val = ga ? calcValue(cpy, s, vv) : calcValue(cpy, Side(int(s)^1), vv);
		val.first = moves[i];
		if(!ga) val.second = 1.0 - val.second;

		if(val.second > bestVal.second) {
			bestVal = val;
		}
	}

	return bestVal;
}

typedef std::unordered_map<Board, float> ValMap;
typedef std::unordered_map<Board, uint8_t> MoveMap;
std::pair<float, float> fillBook(const Board& pos, Side cur, size_t depthLeft, ValMap& sVals, ValMap& nVals, MoveMap& sMoves, MoveMap& noMoves) {
	size_t nMoves;
	const auto* moves = pos.validMoves(cur, nMoves);

	if(depthLeft == 0) {
		if(cur == SOUTH) {
			float val = sVals[pos];
			return std::make_pair(val, 1.0 - val);
		}  else {
			float val = nVals[pos];
			return std::make_pair(1.0 - val, val);
		}
	}
	
	uint8_t bestMove = moves[0];
	float bestVal = -1.0/0.0;
	std::pair<float, float> bestRes;

	for(size_t i = 0; i < nMoves; i++) {
		Board cpy = pos;
		bool ga = cpy.makeMove(cur, moves[i]);
		auto res = fillBook(cpy, ga ? cur : Side(int(cur)^1), depthLeft - 1, sVals, nVals, sMoves, noMoves);

		float scores[2] = { res.first, res.second };
		float ours = scores[int(cur)];

		if(ours > bestVal) {
			bestMove = moves[i];
			bestVal = ours;
			bestRes = res;
		}
	}

	//pie here
	if(cur == NORTH && pos.stonesInWell(NORTH) == 0) {
		noMoves[pos] = 7;
		return std::make_pair(bestRes.second, bestRes.first);
	}

	if(cur == SOUTH) {
		sMoves[pos] = bestMove;
	} else {
		noMoves[pos] = bestMove;
	}

	return bestRes;
}

int main() {
	const size_t depth = 4;
	std::fstream nOut;
	nOut.open("nbook.bin", std::ios::out);
	std::fstream sOut;
	sOut.open("sbook.bin", std::ios::out);

	Board b;
	b.reset();
	
	std::vector<Board> northLeaves;
	std::vector<Board> southLeaves;
	gen_positions_split(depth, SOUTH, b, southLeaves, northLeaves);
	std::vector<float> northValues(northLeaves.size());
	std::vector<float> southValues(southLeaves.size());

	std::cout << southLeaves.size() << " initial south leaves at depth " << depth << std::endl;
	std::cout << northLeaves.size() << " initial north leaves at depth " << depth << std::endl;

	#pragma omp parallel for schedule(dynamic)
	for(size_t i = 0; i < southLeaves.size(); i++) {
		MCAgent gg;
		southValues[i] = gg.makeMoveAndScore(southLeaves[i], SOUTH, 10, 0).second;
	}

	std::cout << "Done calculating values for south leaves" << std::endl;

	#pragma omp parallel for schedule(dynamic)
	for(size_t i = 0; i < northLeaves.size(); i++) {
		MCAgent gg;
		northValues[i] = gg.makeMoveAndScore(northLeaves[i], NORTH, 10, 0).second;
	}

	std::cout << "Done calculating values for north leaves" << std::endl;

	std::unordered_map<Board, float> southMap;
	std::unordered_map<Board, float> northMap;

	for(size_t i = 0; i < southLeaves.size(); i++) {
		southMap[southLeaves[i]] = southValues[i];
	}
	
	for(size_t i = 0; i < northLeaves.size(); i++) {
		northMap[northLeaves[i]] = northValues[i];
	}

	std::cout << "Done inserting values into map" << std::endl;

	std::unordered_map<Board, uint8_t> southFin;
	std::unordered_map<Board, uint8_t> northFin;
	fillBook(b, SOUTH, depth, southMap, northMap, southFin, northFin);

	std::cout << "Done filling books" << std::endl;
	
	// This will break on big endian systems #YOLO
	uint32_t sSize = southFin.size();
	sOut.write((char*)&sSize, sizeof(uint32_t));
	for(const auto& entry : southFin) {
		sOut.write((char*)&entry.first, 16);
		sOut.write((char*)&entry.second, 1);
	}

	std::cout << "Saved " << sSize << " entries for south" << std::endl;

	uint32_t nSize = northFin.size();
	nOut.write((char*)&nSize, sizeof(uint32_t));
	for(const auto& entry : northFin) {
		nOut.write((char*)&entry.first, 16);
		nOut.write((char*)&entry.second, 1);
	}

	std::cout << "Saved " << nSize << " entries for north" << std::endl;
	
	return 0;
}

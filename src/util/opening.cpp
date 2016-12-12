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
	bool firstMove = (whosTurn == SOUTH) && (b.stonesInWell(SOUTH) == 0);
	
	for(size_t i = 0; i < nMoves; i++) {
		Board tmp = b;
		bool again = tmp.makeMove(whosTurn, moves[i]);

		gen_positions_split(depth - 1, (again && !firstMove) ? whosTurn : (Side)(((int)whosTurn)^1), tmp, sb, nb);
	}
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

	bool firstMove = (cur == SOUTH) && (pos.stonesInWell(SOUTH) == 0);
	
	uint8_t bestMove = moves[0];
	float bestVal = -1.0/0.0;
	std::pair<float, float> bestRes;

	for(size_t i = 0; i < nMoves; i++) {
		Board cpy = pos;
		bool ga = cpy.makeMove(cur, moves[i]);
		auto res = fillBook(cpy, (!firstMove && ga) ? cur : Side(int(cur)^1), depthLeft - 1, sVals, nVals, sMoves, noMoves);

		float scores[2] = { res.first, res.second };
		float ours = scores[int(cur)];

		if(ours > bestVal) {
			bestMove = moves[i];
			bestVal = ours;
			bestRes = res;
		}
	}

	//pie here
	if(cur == NORTH && pos.stonesInWell(NORTH) == 0 && bestVal < 0.5) {
		noMoves[pos] = 7;
		nVals[pos] = 1.0 - bestVal;
		return std::make_pair(bestRes.second, bestRes.first);
	}

	if(cur == SOUTH) {
		sVals[pos] = bestVal;
		sMoves[pos] = bestMove;
	} else {
		nVals[pos] = bestVal;
		noMoves[pos] = bestMove;
	}

	return bestRes;
}

int main() {
	const size_t depth = 4;

	Board b;
	b.reset();
	
	std::vector<Board> northLeaves;
	std::vector<Board> southLeaves;
	gen_positions_split(depth, SOUTH, b, southLeaves, northLeaves);
	std::vector<float> northValues(northLeaves.size());
	std::vector<float> southValues(southLeaves.size());

	std::cout << southLeaves.size() << " initial south leaves at depth " << depth << std::endl;
	std::cout << northLeaves.size() << " initial north leaves at depth " << depth << std::endl;
	std::cout << std::endl;

	#pragma omp parallel for schedule(dynamic)
	for(size_t i = 0; i < southLeaves.size(); i++) {
		MCAgent gg;
		southValues[i] = gg.makeMoveAndScore(southLeaves[i], SOUTH, 10, 0).second;

		if(i % 16 == 0) std::cout << "Calculated values for " << 100.0 * float(i)/southLeaves.size() << "% of south leaves" << std::endl;
	}

	std::cout << "Done calculating values for south leaves\n" << std::endl;

	#pragma omp parallel for schedule(dynamic)
	for(size_t i = 0; i < northLeaves.size(); i++) {
		MCAgent gg;
		northValues[i] = gg.makeMoveAndScore(northLeaves[i], NORTH, 10, 0).second;
		
		if(i % 16 == 0) std::cout << "Calculated values for " << 100.0 * float(i)/northLeaves.size() << "% of north leaves" << std::endl;
	}

	std::cout << "Done calculating values for north leaves\n" << std::endl;

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

	std::cout << "Done filling books\n" << std::endl;

	std::fstream nOut;
	nOut.open("nbook.bin", std::ios::out);
	std::fstream sOut;
	sOut.open("sbook.bin", std::ios::out);

	std::fstream nVOut;
	nVOut.open("nvbook.bin", std::ios::out);
	std::fstream sVOut;
	sVOut.open("svbook.bin", std::ios::out);

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

	std::cout << "Saved " << nSize << " entries for north\n" << std::endl;

	sSize = southMap.size();
	sVOut.write((char*)&sSize, sizeof(uint32_t));
	for(const auto& entry : southMap) {
		sVOut.write((char*)&entry.first, 16);
		sVOut.write((char*)&entry.second, sizeof(float));
	}

	std::cout << "Saved " << sSize << " values for south" << std::endl;

	nSize = northMap.size();
	nVOut.write((char*)&nSize, sizeof(uint32_t));
	for(const auto& entry : northMap) {
		nVOut.write((char*)&entry.first, 16);
		nVOut.write((char*)&entry.second, sizeof(float));
	}

	std::cout << "Saved " << nSize << " values for north" << std::endl;
	
	return 0;
}

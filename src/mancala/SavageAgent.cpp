#include "SavageAgent.hpp"

#include "MCAgent.hpp"
#include "MiniMaxAgent.hpp"
#include "books.hpp"

#include <future>
#include <thread>
#include <utility>
#include <iostream>

static std::pair<uint8_t, float> minimaxCheck(size_t movesSoFar, Board b, Side s, double time, std::function<void(uint8_t, double)> up) {
	MiniMaxAgent mm;
	
	if(movesSoFar > 20) {
		Board bCopy = b;
		std::pair<uint8_t, double> result = mm.iterative_deepening(s, bCopy, movesSoFar, time, up);
		if(s == SOUTH && result.second > 200){
			return std::make_pair(result.first, 1.0/0.0);
		} else if(s == NORTH && result.second < -200){
			return std::make_pair(result.first, 1.0/0.0);
		}
	}
	return std::make_pair(0,0.0);
}

static std::pair<uint8_t, float> monteCarloPar(Board b, Side s, size_t movesSoFar, uint8_t lastMove, double time) {
	MCAgent mc(50000000, 1, 1);
	mc.useIterations() = false;
	mc.timePerMove() = time;
	
	return mc.makeMoveAndScore(b, s, movesSoFar, lastMove);
}

uint8_t SavageAgent::makeMove(const Board& b, Side side, size_t movesSoFar, uint8_t lastMove) {
	using namespace std;

	// Swap Logic
	if(movesSoFar == 0) return 1;
	if(movesSoFar == 1 && (lastMove == 1 || lastMove == 2 ||  lastMove == 3 || lastMove == 4 || lastMove == 5 || lastMove == 6)) return 7;
	
	// Opening table
	if(movesSoFar < 4) {
		auto& table = side == SOUTH ? books::southBook() : books::northBook();
		auto it = table.find(b);

		if(it != table.end()) {
			std::cerr << "USING BOOK" << std::endl;
			return it->second;
		} else {
			std::cerr << "BIG PROBLEM" << std::endl;
		}
	}

	double timeForThisMove = std::min(30.0, 300.0/(1.0 + 0.25 * movesSoFar));
	double timeForMM = std::max(10.0, std::min(25.0, 0.571428 * timeForThisMove));

	// Spawn minimax thread
	future<pair<uint8_t, float>> mmFuture;
	volatile uint8_t mmMove;
	volatile double mmScore;
	std::function<void(uint8_t, double)> updater = [&](uint8_t m, double s) { mmMove = m; mmScore = s; };

	{
		std::packaged_task<pair<uint8_t,float>(size_t, Board, Side, double, function<void(uint8_t, double)>)>
			mmTask(minimaxCheck);
		mmFuture = mmTask.get_future();
		thread t(std::move(mmTask), movesSoFar, b, side, timeForMM, updater);
		t.detach();
	}


	size_t nMoves;
	const auto* moves = b.validMoves(side, nMoves);


	//Spawn MC threads
	future<pair<uint8_t, float>> results[7];
	bool ga[7];
	for(size_t i = 0; i < nMoves; i++) {
		Board cpy = b;
		ga[i] = cpy.makeMove(side, moves[i]);
		

		std::packaged_task<pair<uint8_t,float>(Board, Side, size_t, uint8_t, double)>
			mcTask(monteCarloPar);
		
		results[i] = mcTask.get_future();
		thread t(std::move(mcTask), cpy, ga[i] ? side : Side(int(side)^1), movesSoFar, moves[i], timeForThisMove);
		t.detach();

	}

	// Best MC option
	pair<uint8_t, float> best = make_pair(0, -1.0);
	for(size_t i = 0; i < nMoves; i++) {
		auto cur = results[i].get();
		if(!ga[i]) cur.second = 1.0 - cur.second;
		cur.first = moves[i];

		if(cur.second > best.second) {
			best = cur;
		}
	}

	// Waiting for MM
	auto mmRes = mmFuture.get();
	
	// Guaranteed win
	if(mmRes.second > 100000.0) {
		std::cerr << "GUARANTEED WIN" << std::endl;
		return mmRes.first;
	}

	return best.first;
}

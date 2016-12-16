#include "SavageAgent.hpp"

#include "MCAgent.hpp"
#include "MiniMaxAgent.hpp"

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
	MCAgent mc(7, 1, 1);
	mc.useIterations() = false;
	mc.timePerMove() = time;
	
	return mc.makeMoveAndScore(b, s, movesSoFar, lastMove);
}

uint8_t SavageAgent::makeMove(const Board& b, Side side, size_t movesSoFar, uint8_t lastMove) {
	using namespace std;

	double timeForThisMove = 5.0; //300.0/(1.0 + 0.25 * movesSoFar);
	std::cerr << "gonna run for " << timeForThisMove << std::endl;
	
	future<pair<uint8_t, float>> mmFuture;
	volatile uint8_t mmMove;
	volatile double mmScore;
	std::function<void(uint8_t, double)> updater = [&](uint8_t m, double s) { mmMove = m; mmScore = s; };

	{
		std::packaged_task<pair<uint8_t,float>(size_t, Board, Side, double, function<void(uint8_t, double)>)>
			mmTask(minimaxCheck);
		mmFuture = mmTask.get_future();
		thread t(std::move(mmTask), movesSoFar, b, side, 10.0, updater);
		t.detach();
		std::cerr << "started minimax" << std::endl;
	}


	size_t nMoves;
	const auto* moves = b.validMoves(side, nMoves);
	std::cerr << "GONNA SPAWN THREADS FOR " << nMoves << std::endl;

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

		std::cerr << "SPAWNED THREAD " << i << std::endl;
	}

	pair<uint8_t, float> best = make_pair(0, -1.0);
	
	for(size_t i = 0; i < nMoves; i++) {
		auto cur = results[i].get();
		if(!ga[i]) cur.second = 1.0 - cur.second;
		cur.first = moves[i];

		if(cur.second > best.second) {
			best = cur;
		}
	}

	// Waiting
	auto mmRes = mmFuture.get();

	if(mmRes.second > 100000.0) {
		return mmRes.first;
	}

	return best.first;
}

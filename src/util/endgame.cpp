#include <mancala/Board.hpp>
#include <mancala/Game.hpp>

#include <vector>
#include <iostream>
#include <cstring>

struct TinyBoard {
	uint8_t stones[14];
};

void add_sol(std::vector<TinyBoard>& res, uint8_t* sol) {
	TinyBoard b;
	memmove(&b, sol, sizeof(TinyBoard));

	res.push_back(b);
}

void solve(uint8_t* sols, std::vector<TinyBoard>& res, uint8_t N, int idx) {
	if(N == 0) {
		add_sol(res, sols);
		return;
	}

	if(idx >= 14) {
		return;
	}

	for(int i = N; i >= 0; i--) {
		sols[idx] = i;
		solve(sols, res, N-i, idx+1);
	}
}

std::vector<TinyBoard> withNStones(uint8_t N) {
	std::vector<TinyBoard> res;
	
	uint8_t solutions[14] = { 0 };

	solve(solutions, res, N, 0);

	return res;
}

int main() {
	std::vector<std::vector<TinyBoard>> allMoves;
	
	for(int i = 0; i < 16; i++) {
		allMoves.push_back(withNStones(i));
	}

	return 0;
}


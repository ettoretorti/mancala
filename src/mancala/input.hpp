#pragma once

#include <mancala/Board.hpp>

#include <memory>
#include <istream>

namespace input {

struct Message {
	virtual ~Message() {}
};

struct Start : public Message {
	Start(Side s) : side(s) {}
	Side side;
};

struct Change : public Message {
	Change(const Board& b, bool ourTur, uint8_t lastMov) : current(b), ourTurn(ourTur), lastMove(lastMov) {}
	Board current;
	bool ourTurn;
	uint8_t lastMove;
};

struct GameOver : public Message {};

// Useful if/when we switch to non-blocking IO
struct NoInput : public Message {};


std::unique_ptr<Message> parseNext(std::istream& in);

}

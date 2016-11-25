#pragma once

#include "Agent.hpp"

class UserAgent : public Agent {
public:
	uint8_t makeMove(const Board&, Side, size_t, uint8_t) override;
};

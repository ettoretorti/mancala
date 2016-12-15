#include "output.hpp"

namespace output {

static std::string moves[] = {
	"MOVE;1\n",
	"MOVE;2\n",
	"MOVE;3\n",
	"MOVE;4\n",
	"MOVE;5\n",
	"MOVE;6\n",
	"MOVE;7\n",
	"SWAP\n",
};

std::string move(size_t hole) {
	return hole < 7 ? moves[hole] : moves[7];
}

}

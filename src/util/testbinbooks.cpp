#include <mancala/binutils.hpp>

#include <iostream>
#include <fstream>

int main() {
	std::fstream sbook, nbook, svbook, nvbook;
	sbook.open("sbook.bin", std::ios_base::in);
	nbook.open("nbook.bin", std::ios_base::in);
	svbook.open("svbook.bin", std::ios_base::in);
	nvbook.open("nvbook.bin", std::ios_base::in);

	auto sb = binutils::readBook(sbook);
	auto nb = binutils::readBook(nbook);

	auto svb = binutils::readVBook(svbook);
	auto nvb = binutils::readVBook(nvbook);

	Board b;
	b.reset();

	for(uint8_t i = 0; i < 7; i++) {
		Board cpy = b;
		cpy.makeMove(SOUTH, i);

		auto it = nb.find(cpy);
		if(it != nb.end()) {
			std::cout << "When south makes " << int(i) << " north responds with " << int(it->second) << std::endl;
		} else {
			std::cout << "Could not find entry for " << int(i) << std::endl;
		}

		auto vit = nvb.find(cpy);
		if(vit != nvb.end())
			std::cout << "When south makes " << int(i) << " north has value " << vit->second << std::endl;
		else
			std::cout << "Could not find value for " << int(i) << std::endl;
	}

	std::cout << "Book sizes = (" << sb.size() << ", " << nb.size() << ")" << std::endl;
	std::cout << "Value sizes = (" << svb.size() << ", " << nvb.size() << ")" << std::endl;

	return 0;
}

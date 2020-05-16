#include <vector>

#include "common.hpp"

static void run(std::string const&);

int main()
{
	auto const code = parse(std::cin);
	run(code);

	return 0;
}

void run(std::string const &code)
{
	Memory memory(kMemorySize, 0);
	auto dp = &memory[0];

	for (size_t pc = 0; pc < code.size(); ++pc) {
		switch (code[pc]) {
		case '>': ++dp; break;
		case '<': --dp; break;
		case '+': ++*dp; break;
		case '-': --*dp; break;
		case '.': std::cout.put(*dp); break;
		case ',': *dp = std::cin.get(); break;

		case '[':
			if (*dp == 0) {
				auto const saved_pc = pc;

				auto depth = 1;
				while (depth != 0 && ++pc < code.size()) {
					switch (code[pc]) {
					case '[': ++depth; break;
					case ']': --depth; break;
					}
				}

				if (depth != 0) {
					std::cerr << "Unmatched '[' at pc=" << saved_pc << std::endl;
					std::exit(EXIT_FAILURE);
				}
			}

			break;

		case ']':
			if (*dp != 0) {
				auto const saved_pc = pc;

				auto depth = -1;
				while (depth != 0 && pc > 0) {
					switch (code[--pc]) {
					case '[': ++depth; break;
					case ']': --depth; break;
					}
				}

				if (depth != 0) {
					std::cerr << "Unmatched ']' at pc=" << saved_pc << std::endl;
					std::exit(EXIT_FAILURE);
				}
			}

			break;
		}
	}
}

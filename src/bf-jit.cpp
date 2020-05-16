#include <vector>
#include <stack>

#include "common.hpp"
#include "../xbyak/xbyak.h"

// putchar() と getchar() がマクロで定義されている場合をケア
static void myputchar(char c) { putchar(c); }
static char mygetchar() { return getchar(); }

struct JittedBF
	: public Xbyak::CodeGenerator
{
	JittedBF(std::string const&, Memory const&);

	void run()
	{
		auto const code = (void(*)())getCode();
		code();
	}
};

int main()
{
	Memory memory(kMemorySize, 0);

	auto const program = parse(std::cin);
	JittedBF(program, memory).run();

	return 0;
}

JittedBF::JittedBF(std::string const &program, Memory const &memory)
	: CodeGenerator(kCodeSize)
{
	using namespace Xbyak;

	size_t pc = 0;
	std::stack<std::pair<Label, Label>> label_stack;

	auto const dp    = rbx;
	auto const value = byte[dp];

	push(dp);
	mov(dp, (uintptr_t)&memory[0]);

	auto const num_repeats = [&](char c)
	{
		auto const saved_pc = pc++;

		while (pc < program.size() && program[pc] == c) {
			++pc;
		}

		return pc-- - saved_pc;
	};

	for (; pc < program.size(); ++pc) {
		switch (program[pc]) {
		case '>': add(dp, num_repeats('>')); break;
		case '<': sub(dp, num_repeats('<')); break;
		case '+': add(value, num_repeats('+')); break;
		case '-': sub(value, num_repeats('-')); break;
		case '.': mov(rdi, value); call(myputchar); break;
		case ',': call(mygetchar); mov(value, rax); break;

		case '[':
			{
				auto label_pair = std::make_pair(L(), Label{});
				cmp(value, 0);
				jz(label_pair.second, CodeGenerator::T_NEAR);

				label_stack.push(label_pair);
			}

			break;

		case ']':
			{
				if (label_stack.empty()) {
					std::cout << "Unmatched ']' at pc=" << pc << std::endl;
					std::exit(EXIT_FAILURE);
				}

				auto label_pair = label_stack.top();
				label_stack.pop();

				cmp(value, 0);
				jnz(label_pair.first);
				L(label_pair.second);
			}

			break;
		}
	}

	pop(dp);
	ret();
}

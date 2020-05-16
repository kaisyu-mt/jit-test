#include <vector>
#include <stack>

#include "common.hpp"

using Instruction = std::pair<int, int>;
using Code = std::vector<Instruction>;

enum
{
	kEnd = 0,

	kIncPtr, kDecPtr,
	kIncValue, kDecValue,
	kPutChar, kGetChar,
	kJumpForward, kJumpBack,

	kSetZero,   // e.g. [-]
	kMovePtr,   // e.g. [<<<]
	kMoveValue, // e.g. [-<<<+>>>]
};

static void run(Code const&);
static Code optimize(std::string const&);
static Code optimizeLoop(Code const&, size_t);

int main()
{
	auto const program = parse(std::cin);
	auto const code = optimize(program);
	run(code);

	return 0;
}

void run(Code const &code)
{
	Memory memory(kMemorySize, 0);
	auto dp = &memory[0];

	for (size_t pc = 0; ; ++pc) {
		auto op = code[pc];

		switch (op.first) {
		case kEnd: return;
		case kIncPtr: dp += op.second; break;
		case kDecPtr: dp -= op.second; break;
		case kIncValue: *dp += op.second; break;
		case kDecValue: *dp -= op.second; break;
		case kPutChar: for (int i = 0; i < op.second; ++i) { std::cout.put(*dp); } break;
		case kGetChar: for (int i = 0; i < op.second; ++i) { *dp = std::cin.get(); } break;
		case kSetZero: *dp = 0; break;
		case kMovePtr: while (*dp != 0) { dp += op.second; } break;
		case kMoveValue: if (*dp != 0) { dp[op.second] += *dp; *dp = 0; } break;
		case kJumpForward: if (*dp == 0) { pc = op.second; } break;
		case kJumpBack: if (*dp != 0) { pc = op.second; } break;
		}
	}
}

Code optimize(std::string const &src)
{
	Code iseq;
	std::stack<size_t> lbrack_stack;

	for (size_t pc = 0; pc < src.size();) {
		auto const c = src[pc];

		if (c == '[') {
			lbrack_stack.push(iseq.size());
			iseq.push_back({kJumpForward, 0});

			++pc;

		} else if (c == ']') {
			if (lbrack_stack.empty()) {
				std::cerr << "Unmatched ']' at pc=" << pc << std::endl;
				std::exit(EXIT_FAILURE);
			}

			auto const offset = lbrack_stack.top();
			lbrack_stack.pop();

			auto optimized_loop = optimizeLoop(iseq, offset);
			if (optimized_loop.empty()) {
				iseq[offset].second = iseq.size(); // '[' のジャンプ先を決定
				iseq.push_back({kJumpBack, offset});
			} else {
				iseq.erase(iseq.begin() + offset, iseq.end());
				iseq.insert(iseq.end(), optimized_loop.begin(), optimized_loop.end());
			}

			++pc;

		} else {
			size_t num_repeats = 1;
			while (pc < src.size() && src[++pc] == c) {
				++num_repeats;
			}

			switch (c) {
			case '>': iseq.push_back({kIncPtr, num_repeats}); break;
			case '<': iseq.push_back({kDecPtr, num_repeats}); break;
			case '+': iseq.push_back({kIncValue, num_repeats}); break;
			case '-': iseq.push_back({kDecValue, num_repeats}); break;
			case '.': iseq.push_back({kPutChar, num_repeats}); break;
			case ',': iseq.push_back({kGetChar, num_repeats}); break;
			}
		}
	}

	iseq.push_back({kEnd, 0});
	return iseq;
}

Code optimizeLoop(Code const &iseq, size_t loop_start)
{
	Code new_loop;

	auto const length = iseq.size() - loop_start;
	auto const loop = iseq.cbegin() + loop_start; // '['

	if (length == 2) {
		int op = loop[1].first;
		if (op == kIncValue || op == kDecValue) { // [-] or [+]
			new_loop.push_back({kSetZero, 0});
		} else if (op == kIncPtr || op == kDecPtr) { // [<] or [>]
			int arg = loop[1].second;
			new_loop.push_back({kMovePtr, op == kIncPtr ? arg : -arg});
		}
	} else if (length == 5) { // [-<+>] or [->+<]
		if (loop[1].first == kDecValue && loop[3].first == kIncValue &&
			loop[1].second == 1 && loop[3].second == 1 &&
			loop[2].second == loop[4].second)
		{
			if (loop[2].first == kIncPtr && loop[4].first == kDecPtr) {
				new_loop.push_back({kMoveValue, loop[2].second});
			} else if (loop[2].first == kDecPtr && loop[4].first == kIncPtr) {
				new_loop.push_back({kMoveValue, -loop[2].second});
			}
		}
	}

	return new_loop;
}

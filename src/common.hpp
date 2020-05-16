#pragma once

#include <string>
#include <iostream>

constexpr size_t kMemorySize = 30000;
constexpr size_t kCodeSize = 50000;

using Memory = std::vector<uint8_t>;

std::string parse(std::istream &input)
{
	std::string program;

	for (std::string line; std::getline(input, line);) {
		for (auto const c: line) {
			switch (c) {
			case '>': case '<':
			case '+': case '-':
			case '.': case ',':
			case '[': case ']':
				program.push_back(c);
				break;
			}
		}
	}

	return program;
}

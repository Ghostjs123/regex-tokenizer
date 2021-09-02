#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <stdio.h>
#include <sstream>
#include <algorithm>
#include "token.h"
#include "regex-tokenizer.h"
#include "util.h"
#include "unit-testing-util.h"

int main(int argc, char* argv[]) {
	if (argc == 1) {
		std::cout << "Missing input filename\n";
		return 0;
	}

	if (!file_exists(argv[1])) {
		std::cout
			<< "No file named \""
			<< argv[1]
			<< "\""
			<< std::endl;
		return 0;
	}

	std::vector<std::string> contents = read_lines(argv[1]);

	Tokenizer tokenizer(contents);

	tokenizer.print();

	if (argc == 3 && argv[2] == (std::string)"-c") {
		(void)compare_tokenization_results(argv[1], false);
	}

	return 0;
}
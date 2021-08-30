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

// https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
std::string execute_python_tokenizer(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}

std::vector<int> as_char_codes(std::string line) {
	std::vector<int> codes;

	for (char c : line) {
		codes.push_back(+c);
	}

	return codes;
}

std::vector<std::string> split_newline(char* result) {
	std::vector<std::string> lines;
	std::stringstream ss(result);
	std::string to;

	if (result != NULL) {
		while (getline(ss, to, '\n')) {
			lines.push_back(to);
		}
	}

	return lines;
}

int codes_as_int(std::vector<int> codes) {
	std::string temp = "";

	for (int i : codes) {
		temp += char(i);
	}

	return std::stoi(temp);
}

std::tuple<int, int, int, int> parse_position(std::vector<int> codes) {
	// ex: 1,0-1,2     except char codes
	std::vector<int>::iterator iter;
	iter = find(codes.begin(), codes.end(), +',');
	int first_comma = std::distance(codes.begin(), iter);
	iter = find(codes.begin(), codes.end(), +'-');
	int dash = std::distance(codes.begin(), iter);
	iter = find(codes.begin()+dash, codes.end(), +',');
	int second_comma = std::distance(codes.begin(), iter);

	std::tuple<int, int, int, int> result;
	// line start
	std::get<0>(result) = codes_as_int(
		std::vector<int>(codes.begin(), codes.begin()+first_comma)
	);
	// column start
	std::get<1>(result) = codes_as_int(
		std::vector<int>(codes.begin()+first_comma+1, codes.begin()+dash)
	);
	// line end
	std::get<2>(result) = codes_as_int(
		std::vector<int>(codes.begin()+dash+1, codes.begin()+second_comma)
	);
	// column end
	std::get<3>(result) = codes_as_int(
		std::vector<int>(codes.begin()+second_comma+1, codes.end())
	);
	return result;
}

bool not_space(int i) {
	return i != +' ';
}

std::string codes_as_string(std::vector<int> codes) {
	std::string result = "";

	for (int i : codes) {
		result += char(i);
	}
	return result;
}

std::vector<Token> parse_tokens(std::string result) {
	std::vector<Token> python_tokens;
	std::vector<std::string> lines = split_newline(&result[0]);

	int colon = +':';
	int apostrophe = +'\'';
	int space = +' ';

	std::vector<int> codes;
	std::vector<int>::iterator iter;

	for (std::string s : lines) {
		codes = as_char_codes(s);
		
		// determine the position of the token
		iter = find(codes.begin(), codes.end(), colon);
		int pos_end = distance(codes.begin(), iter);
		std::tuple<int, int, int, int> position = parse_position(
			std::vector<int>(codes.begin(), codes.begin()+pos_end)
		);
		std::tuple<int, int> pos1(std::get<0>(position), std::get<1>(position));
		std::tuple<int, int> pos2(std::get<2>(position), std::get<3>(position));

		// determine the type of the token
		iter = find_if(codes.begin()+pos_end+1, codes.end(), not_space);
		int type_start = distance(codes.begin(), iter);
		iter = find(codes.begin()+type_start, codes.end(), space);
		int type_end = distance(codes.begin(), iter);
		std::string type = codes_as_string(
			std::vector<int>(codes.begin()+type_start, codes.begin()+type_end)
		);

		// determine the value of the token
		iter = find(codes.begin()+type_end, codes.end(), apostrophe);
		int value_start = distance(codes.begin(), iter);
		iter = find(codes.begin()+value_start+1, codes.end(), apostrophe);
		int value_end = distance(codes.begin(), iter);
		std::string value = codes_as_string(
			std::vector<int>(codes.begin()+value_start+1, codes.begin()+value_end)
		);

		// add token
		python_tokens.push_back(Token(type, value, pos1, pos2));
	}
	return python_tokens;
}

void print_mismatch(
	Tokenizer tokenizer,
	Token prev, Token curr,
	std::vector<Token> python_tokens,
	int i
) {
	std::cout << std::endl << "Tokens did not match on line: " << curr.line_start << std::endl;
	std::cout << "mine:" << std::endl;

	if (prev != Token()) {
		std::cout
			<< prev << std::endl 
			<< curr << std::endl 
			<< tokenizer.next_token() << std::endl;
	}
	else {
		std::cout
			<< curr << std::endl 
			<< tokenizer.next_token() << std::endl;
	}

	std::cout << "theirs:" << std::endl;

	if (i > 0) {
		std::cout
			<< python_tokens.at(i-1) << std::endl 
			<< python_tokens.at(i) << std::endl 
			<< python_tokens.at(i+1) << std::endl;
	}
	else {
		std::cout
			<< python_tokens.at(i) << std::endl
			<< python_tokens.at(i+1) << std::endl;
	}
}

void compare_tokens(std::string fname, Tokenizer tokenizer) {
	std::string cmd = "python3 -m tokenize " + fname;
	std::string result = execute_python_tokenizer(cmd.c_str());
	std::vector<Token> python_tokens = parse_tokens(result);
	python_tokens.erase(python_tokens.begin());  // delete encoding token

	Token t = python_tokens.at(0);  // ignore initial value
	Token prev = Token();
	for (int i=0; i < python_tokens.size(); i++) {
		if (python_tokens.at(i) != (t = tokenizer.next_token())) {
			print_mismatch(tokenizer, prev, t, python_tokens, i);
			std::cout 
				<< std::endl
				<<"Tokens DID NOT match 'python -m tokenize'"
				<< std::endl;
			return;
		}
		prev = t;
	}
	std::cout << std::endl << "Tokens match 'python -m tokenize'" << std::endl;
}

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

    std::cout << "tokens:" << std::endl;
	tokenizer.print();

	if (argc == 3 && argv[2] == (std::string)"-c") {
		compare_tokens(argv[1], tokenizer);
	}

	return 0;
}
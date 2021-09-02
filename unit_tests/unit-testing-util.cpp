#include <string>
#include <array>
#include <iostream>
#include <memory>
#include <vector>
#include <sstream>

// https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
std::string exec_command(const char* cmd) {
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

std::vector<std::string> split_on_newline(char* str) {
	std::vector<std::string> lines;
	std::stringstream ss(str);
	std::string to;

	if (str != NULL) {
		while (std::getline(ss, to, '\n')) {
			lines.push_back(to);
		}
	}

	return lines;
}

void print_mismatch(
	const std::vector<std::string>& python_tokenizer_results,
	const std::vector<std::string>& regex_tokenizer_results,
	int i
) {
    std::cout << "regex-tokenizer-main" << std::endl;
	if (i > 0) {
		std::cout
			<< regex_tokenizer_results.at(i-1) << std::endl
			<< regex_tokenizer_results.at(i) << std::endl
			<< regex_tokenizer_results.at(i+1) << std::endl;
	}
	else {
		std::cout
			<< regex_tokenizer_results.at(i) << std::endl
			<< regex_tokenizer_results.at(i+1) << std::endl;
	}

	std::cout << "python -m tokenize" << std::endl;
	if (i > 0) {
		std::cout
			<< python_tokenizer_results.at(i-1) << std::endl
			<< python_tokenizer_results.at(i) << std::endl
			<< python_tokenizer_results.at(i+1) << std::endl;
	}
	else {
		std::cout
			<< python_tokenizer_results.at(i) << std::endl
			<< python_tokenizer_results.at(i+1) << std::endl;
	}
}

/**
 *  @brief Compares the output of 'python -m tokenize {fname}' and 'regex-tokenizer-main {fname}'.
 *  @param fname file to compare tokenization against.
 *  @param silent toggles printing to std::cout on/off.
 *  @returns true if tokenization results match, else false.
**/
bool compare_tokenization_results(const std::string& fname, bool silent=false) {
	std::string result1 = exec_command(
        ("python3 -m tokenize " + fname).c_str()
    );
    std::vector<std::string> python_tokenizer_results = split_on_newline(&result1[0]);
    std::string result2 = exec_command(
        ("./regex-tokenizer-main " + fname).c_str()
    );
    std::vector<std::string> regex_tokenizer_results = split_on_newline(&result2[0]);

	for (int i=0; i < (int)python_tokenizer_results.size(); i++) {
		if (python_tokenizer_results.at(i) != regex_tokenizer_results.at(i)) {
            if (!silent) {
                std::cout << std::endl;
                print_mismatch(
                    python_tokenizer_results,
                    regex_tokenizer_results,
                    i
                );
                std::cout
                    << std::endl
                    <<"Tokens DID NOT match 'python -m tokenize'"
                    << std::endl;
            }
			return false;
		}
	}
    if (!silent) {
        std::cout
            << std::endl
            << "Tokens match 'python -m tokenize'"
            << std::endl;
    }
    return true;
}

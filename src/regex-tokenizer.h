#ifndef REGEX_TOKENIZER_H
#define REGEX_TOKENIZER_H

#include <string>
#include <vector>
#include <tuple>
#include <regex>
#include "token.h"

/**
 *  @brief Tokenizes a given vector<string> input.
**/
class Tokenizer {
    private:
        int pos;
        std::vector<std::tuple<std::string, std::regex>> regexs;
        std::regex s_regex_three_double_quotes_same_line;
        std::regex s_regex_three_single_quotes_same_line;
        std::vector<std::string> input;
        std::vector<Token> tokens;

        // tokenize utilities
        void clear();
        void build_regexs();
        std::tuple<std::string, std::string> apply_regexs(std::string& line);
        std::regex get_string_close_regex(std::string type);
        int check_string_termination(std::string line, std::regex close_regex);
        int lstrip_spaces(int line_number, int current_pos);

        // main tokenization function
        void tokenize();

        // Token push functions
        void push_encoding();
        void push_newline(int line_number, int current_pos);
        void push_nl(int line_number, int current_pos);
        void push_token(
            std::string type,
            std::string value,
            std::tuple<int, int> start,
            std::tuple<int, int> end
        );
        void push_indent(std::string value, int line_number);
        void push_dedent(int line_number);
        void push_eof(std::vector<int> indents, int line_number);

    public:
        Tokenizer(std::vector<std::string> input);

        Token at(int i);
        Token next_token();
        void print();
};

#endif

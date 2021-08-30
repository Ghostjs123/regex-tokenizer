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
        std::vector<std::string> input;
        std::vector<Token> tokens;

        // tokenize utilities
        void clear();
        void build_regexs();
        std::tuple<std::string, std::string> apply_regexs(std::string& line);
        int lstrip_spaces(int line_number, int current_pos);

        // main tokenization function
        void tokenize();

        // Token push functions
        void push_newline(int line_number, int current_pos);
        void push_nl(int line_number, int current_pos);
        void push_token(
            std::string type,
            std::string value,
            int line_number,
            int current_pos
        );
        void push_indent(std::string value, int line_number);
        void push_dedent(int line_number);
        void push_eof();

    public:
        Tokenizer(std::vector<std::string> input);

        Token next_token();
        void print();
};

#endif

#ifndef REGEX_TOKENIZER_H
#define REGEX_TOKENIZER_H

#include <string>
#include <vector>
#include "token.h"

class Tokenizer {
    private:
        int pos;
        int length;
        std::vector<std::string> input;
        std::vector<Token> tokens;

        void tokenize();

    public:
        Tokenizer(std::vector<std::string> input);

        Token next_token();
        void print();
};

#endif

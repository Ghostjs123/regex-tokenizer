#include <iostream>
#include <vector>
#include <regex>
#include <string>
#include <tuple>
#include "logging.h"
#include "util.h"
#include "token.h"
#include "regex-tokenizer.h"

Tokenizer::Tokenizer(std::vector<std::string> input) {
    this->pos = 0;
    this->length = 0;
    this->input = input;

    this->tokenize();
}

void Tokenizer::tokenize() {

}

Token Tokenizer::next_token() {
    if (this->pos < this->length) {
        return this->tokens[this->pos++];
    }
    throw std::runtime_error("next_token() with no tokens remaining");
}

void Tokenizer::print() {
    for (Token t : this->tokens) {
        std::cout << t << std::endl;
    }
}

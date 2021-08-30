#include <iostream>
#include <vector>
#include <regex>
#include <string>
#include <tuple>
#include <numeric>
#include "logging.h"
#include "util.h"
#include "token.h"
#include "regex-tokenizer.h"

// NOTE: source on how python handles indentation
// https://docs.python.org/2.0/ref/indentation.html

// Specifically:
/* Before the first line of the file is read, a single zero is pushed on the stack; 
   this will never be popped off again. The numbers pushed on the stack will always 
   be strictly increasing from bottom to top. At the beginning of each logical line, 
   the line's indentation level is compared to the top of the stack. If it is equal, 
   nothing happens. If it is larger, it is pushed on the stack, and one INDENT token 
   is generated. If it is smaller, it must be one of the numbers occurring on the stack; 
   all numbers on the stack that are larger are popped off, and for each number popped 
   off a DEDENT token is generated. At the end of the file, a DEDENT token is generated 
   for each number remaining on the stack that is larger than zero.
*/
// This is incredibly important when it comes to parsing productions like:
// block:
//     | NEWLINE INDENT statements DEDENT 
//     | simple_stmt

// implementation of tokenization logic written in python
// https://github.com/python/cpython/blob/85fd9f4e45ee95e2608dbc8cc6d4fe28e4d2abc4/Lib/tokenize.py#L45

// python3 -m tokenize [filename]

// NOTE: _sub and sub are neeed because c++'s builtin .substr() method
// does not like carriage returns '\r' at the end of the string
std::string _sub(std::string s, int prev, int i) {
    std::string sn = "";

    for (int _i=prev; _i < i; _i++) {
        sn += s[_i];
    }

    return sn;
}
std::string sub(std::string s, int prev, int i) {
    if (i-prev == 1 || i == prev) {
        return std::string(1, s[prev]);
    }
    return _sub(s, prev, i);
}

/**
 *  @brief Resets the state of the Tokenizer.
**/
void Tokenizer::clear() {
    if (this->input.size() > 0) {
        this->input.clear();
    }
    if (this->tokens.size() > 0) {
        this->tokens.clear();
    }
    this->pos = 0;
}

/**
 *  @brief Tokenizer constructor. Tokenizes the input vector.
 *  @param input input to tokenize.
**/
Tokenizer::Tokenizer(std::vector<std::string> input) {
    this->clear();
    this->input = input;

    this->build_regexs();
    this->tokenize();
}

/**
 *  @brief Initializes this->regexs.
**/
void Tokenizer::build_regexs() {
    this->regexs = {
        {"OP", std::regex("^(\\()")},
        {"OP", std::regex("^(\\))")},
        {"OP", std::regex("^(\\[)")},
        {"OP", std::regex("^(\\])")},
        {"OP", std::regex("^(\\{)")},
        {"OP", std::regex("^(\\})")},
        {"OP", std::regex("^(==)")},
        {"OP", std::regex("^(=)")},
        {"OP", std::regex("^(:)")},
        {"OP", std::regex("^(\\+)")},
        {"OP", std::regex("^(-)")},
        {"OP", std::regex("^(\\*\\*)")},
        {"OP", std::regex("^(\\*)")},
        {"OP", std::regex("^(//)")},
        {"OP", std::regex("^(/)")},
        {"THREE_DOUBLE_QUOTES", std::regex("^(\"\"\")")},
        {"THREE_SINGLE_QUOTES", std::regex("^(''')")},
        {"DOUBLE_QUOTES", std::regex("^(\")")},
        {"SINGLE_QUOTES", std::regex("^(')")},
        {"COMMENT", std::regex("^(#.*)")},
        {"NUMBER", std::regex("^(-?[0-9.,]+)")},  // NOTE: using + and not *
        {"NAME", std::regex("^([a-zA-Z0-9_]+)")}  // NOTE: using + and not *
    };
}

/**
 *  @brief Applies all regexs from this->regexs until a match is found.
 *  @param line string to apply this->regexs against.
 *  @returns The match like {type, value} as a tuple.
**/
std::tuple<std::string, std::string> Tokenizer::apply_regexs(std::string& line) {
    std::smatch match;
    std::string match_result;

    for (auto regex_tuple : this->regexs) {
        if (std::regex_search(line, match, std::get<1>(regex_tuple))) {
            match_result = match[0];
            line.erase(0, ((std::string)match[0]).size());  // NOTE: this erases match[0] as well

            return {std::get<0>(regex_tuple), match_result};
        }    
    }

    throw std::runtime_error("No regex matched: " + line);
}

/**
 *  @brief Left strips whitespace from the current line in this->input.
 *  @param line_number current line being parsed.
 *  @param current_pos current position in the line.
 *  @returns The amount of stripped characters.
**/
int Tokenizer::lstrip_spaces(int line_number, int current_pos) {
    int next_position = this->input[line_number].find_first_not_of(" ");

    if (next_position == std::string::npos) {
        return 0;
    }

    this->input[line_number].erase(0, next_position);

    return next_position;
}

/**
 *  @brief Main function to kick off tokenization.
**/
void Tokenizer::tokenize() {
    std::vector<int> indents{0};  // NOTE: pushing the single 0 mentioned in the comments above

    // NOTE: flag for multi-line strings
    bool in_string = false;

    for (int line_number=0; line_number < this->input.size(); line_number++) {
        // NOTE: intentionally ommiting '\r' and '\n'
        int current_pos = this->input[line_number].find_first_not_of("\t ");

        // NOTE: check for empty line
        if (input[line_number].size() == 0) {
            // NOTE: found an empty line
            this->push_nl(line_number);
            continue;
        }

        // NOTE: check for comments
        if (input[line_number][current_pos] == '#') {
            // NOTE: found a comment
            this->push_token(
                "COMMENT",
                input[line_number].substr(current_pos),
                line_number,
                current_pos
            );
            continue;
        }
        
        // NOTE: handle indentation
        // NOTE: identation logic needs to be skipped on empty lines and comments
        if (current_pos > indents.back()) {
            // NOTE: indentation level increasing
            indents.push_back(current_pos);
            this->push_indent(
                sub(this->input[line_number], 0, current_pos),
                line_number
            );
            this->input[line_number].erase(0, current_pos);  // NOTE: erase indent
        }
        else if (current_pos < indents.back()) {
            // NOTE: indentation level decreasing
            while (current_pos < indents.back()) {
                indents.pop_back();
                this->push_dedent(line_number);
            }

            if (current_pos != indents.back()) {
                throw std::runtime_error(
                    "line " + std::to_string(line_number+1) +
                    "\nunindent does not match any outer indentation level"
                );
            }
        }

        // NOTE: tokenize the line
        while (this->input[line_number].size() > 0) {
            auto next_match = this->apply_regexs(this->input[line_number]);

            this->push_token(
                std::get<0>(next_match),
                std::get<1>(next_match),
                line_number,
                current_pos
            );
            current_pos += std::get<1>(next_match).size();

            current_pos += this->lstrip_spaces(line_number, current_pos);
        }

        // NOTE: done tokenizing the line, push a NEWLINE
        this->push_newline(line_number, current_pos);
    }

    // NOTE: done tokenizing the file, cleanup and push ENDMARKER
    this->push_eof();
}

/**
 *  @brief Pushes a Token based on inputs to this->tokens.
 *  @param type Token type.
 *  @param value Token value.
 *  @param line_number current line number being tokenized.
 *  @param current_pos current position in the line.
**/
void Tokenizer::push_token(
    std::string type,
    std::string value,
    int line_number,
    int current_pos
) {
    this->tokens.push_back(
        Token(
            type,
            value,
            {line_number+1, current_pos},
            {line_number+1, current_pos+value.size()}
        )
    );
}

/**
 *  @brief Pushes an INDENT Token based on inputs to this->tokens.
 *  @param value Token value.
 *  @param line_number current line number being tokenized.
**/
void Tokenizer::push_indent(std::string value, int line_number) {
    this->tokens.push_back(
        Token(
            "INDENT",
            value,
            {line_number+1, 0},
            {line_number+1, value.size()}
        )
    );
}

/**
 *  @brief Pushes a DEDENT Token based on inputs to this->tokens.
 *  @param line_number current line number being tokenized.
**/
void Tokenizer::push_dedent(int line_number) {
    this->tokens.push_back(
        Token(
            "DEDENT",
            "",
            {line_number+1, 0},
            {line_number+1, 1}
        )
    );
}

/**
 *  @brief Pushes a NEWLINE Token to this->tokens.
 *  @param line_number current line number being tokenized.
 *  @param current_pos current position in the line.
**/
void Tokenizer::push_newline(int line_number, int current_pos) {
    this->tokens.push_back(
        Token(
            "NEWLINE",
            "\\n",
            {line_number+1, current_pos},
            {line_number+1, current_pos+1}
        )
    );
}

/**
 *  @brief Pushes a NL Token to this->tokens.
 *  @param line_number current line number being tokenized.
**/
void Tokenizer::push_nl(int line_number) {
    this->tokens.push_back(
        Token(
            "NL",
            "\\n",
            {line_number+1, 0},
            {line_number+1, 1}
        )
    );
}

/**
 *  @brief Pushes an ENDMARKER Token to this->tokens.
**/
void Tokenizer::push_eof() {

}

/**
 *  @brief Returns the next Token from this->tokens. Increments this->pos.
 *  @returns The next Token.
**/
Token Tokenizer::next_token() {
    if (this->pos < this->tokens.size()) {
        return this->tokens[this->pos++];
    }
    throw std::runtime_error("next_token() with no tokens remaining");
}

/**
 *  @brief Prints all tokens in this->tokens to std::cout.
**/
void Tokenizer::print() {
    for (Token t : this->tokens) {
        std::cout << t << std::endl;
    }
}

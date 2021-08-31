#include <iostream>
#include <vector>
#include <regex>
#include <string>
#include <tuple>
#include <numeric>
#include <cassert>
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
    // NOTE: there is baked in top to bottom precedence here (top is highest)
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
        // NOTE: the following 4 STRING regex's handle strings that terminate on the same line
        {"STRING", std::regex("^(\"\"\".*\"\"\")")},
        {"STRING", std::regex("^('''.*''')")},
        {"STRING", std::regex("^(r?b?\".*\")")},  // NOTE: strings can be like rb"" or just ""
        {"STRING", std::regex("^(r?b?'.*')")},  // NOTE: strings can be like rb'' or just ''
        // NOTE: these next 2 regex's are for multiline strings
        {"THREE_DOUBLE_QUOTES", std::regex("^(\"\"\")")},
        {"THREE_SINGLE_QUOTES", std::regex("^(''')")},
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

std::regex Tokenizer::get_string_close_regex(std::string type) {
    if (type == "THREE_DOUBLE_QUOTES") {
        return std::regex("\"\"\"");
    }
    else if (type == "THREE_DOUBLE_QUOTES") {
        return std::regex("'''");
    }
    throw std::runtime_error(
        "Unhandled type '" +
        type +
        "' in Tokenizer::get_string_close_regex"
    );
}

int Tokenizer::check_string_termination(std::string line, std::regex close_regex) {
    return -1;
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

// NOTE: this function lines up pretty well with the _tokenize function from
// https://github.com/python/cpython/blob/85fd9f4e45ee95e2608dbc8cc6d4fe28e4d2abc4/Lib/tokenize.py#L45
// I'm borrowing some structure/logic from it to make sure my tokenization is 1:1
/**
 *  @brief Main function to kick off tokenization.
**/
void Tokenizer::tokenize() {
    std::vector<int> indents{0};  // NOTE: pushing the single 0 mentioned in the comments above

    // NOTE: flag for multi-line strings
    bool in_string = false;
    std::string string_value;
    std::string string_close_value;
    std::tuple<int, int> string_start;
    std::regex string_close_regex;
    // NOTE: counter for opening/closing ([{
    int paren_level = 0;
    const std::vector<char> open_parens{'(', '[', '{'};
    const std::vector<char> close_parens{')', ']', '}'};

    for (int line_number=0; line_number < this->input.size(); line_number++) {
        // NOTE: intentionally ommiting '\r' and '\n'
        int current_pos = this->input[line_number].find_first_not_of("\t ");

        if (in_string) {
            // NOTE: checking for the termination of the current multiline string
            int termination_pos = this->check_string_termination(
                this->input[line_number],
                string_close_regex
            );

            if (termination_pos != -1) {
                // NOTE: string terminates on this line
                string_value += this->input[line_number].substr(0, termination_pos);
                current_pos = termination_pos;
                this->input[line_number].erase(0, termination_pos);
                this->push_token(
                    "STRING",
                    string_value,
                    string_start,
                    {line_number+1, current_pos}
                );
                in_string = false;
            }
            else {
                // NOTE: this line belongs to the current multiline string
                string_value += "\n" + this->input[line_number];
                continue;
            }
        }

        else if (paren_level == 0) {
            if (input[line_number].size() == 0) {
                // NOTE: found an empty line
                this->push_nl(line_number, 0);
                continue;
            }
            else if (input[line_number][current_pos] == '#') {
                // NOTE: found a comment
                this->push_token(
                    "COMMENT",
                    input[line_number].substr(current_pos),
                    line_number,
                    current_pos
                );
                continue;
            }

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
        }

        // NOTE: tokenize the line
        while (this->input[line_number].size() > 0) {
            auto next_match = this->apply_regexs(this->input[line_number]);

            if (
                std::get<0>(next_match) == "NUMBER" ||
                std::get<0>(next_match) == "COMMENT" ||
                std::get<0>(next_match) == "NAME" ||
                std::get<0>(next_match) == "STRING"
            ) {
                // NOTE: default Tokens, no extra work needed
                this->push_token(
                    std::get<0>(next_match),
                    std::get<1>(next_match),
                    line_number,
                    current_pos
                );
            }
            else if (
                std::get<0>(next_match) == "THREE_DOUBLE_QUOTES" ||
                std::get<0>(next_match) == "THREE_SINGLE_QUOTES"
            ) {
                // NOTE: multiline string starting
                string_close_regex = this->get_string_close_regex(std::get<0>(next_match));
                string_start = {line_number+1, current_pos};
                string_value = std::get<1>(next_match);
                string_close_value = std::get<1>(next_match);
                current_pos += std::get<1>(next_match).size();
                in_string = true;
            }
            else {
                // NOTE: anything that is not an OP should be handled above
                assert(std::get<0>(next_match) == "OP");

                // NOTE: check for opening/closing characters
                if (
                    std::find(
                        std::begin(open_parens),
                        std::end(open_parens),
                        this->input[line_number][current_pos]
                    ) != open_parens.end()
                ) {
                    paren_level++;
                }
                else if (
                    std::find(
                        std::begin(close_parens),
                        std::end(close_parens),
                        this->input[line_number][current_pos]
                    ) != close_parens.end()
                ) {
                    paren_level--;
                }

                this->push_token(
                    std::get<0>(next_match),
                    std::get<1>(next_match),
                    line_number,
                    current_pos
                );
            }

            current_pos += std::get<1>(next_match).size();
            // NOTE: skipping whitespace between tokens
            current_pos += this->lstrip_spaces(line_number, current_pos);
        }

        // NOTE: done tokenizing the line, push a NL/NEWLINE
        if (paren_level > 0) {
            this->push_nl(line_number, current_pos);
        } else {
            this->push_newline(line_number, current_pos);
        }
        
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
 *  @brief Pushes a Token based on inputs to this->tokens.
 *  @param type Token type.
 *  @param value Token value.
 *  @param start starting line and column.
 *  @param end ending line and column.
**/
void Tokenizer::push_token(
    std::string type,
    std::string value,
    std::tuple<int, int> start,
    std::tuple<int, int> end
) {
    this->tokens.push_back(
        Token(
            type,
            value,
            start,
            end
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
 *  @param current_pos current position in the line.
**/
void Tokenizer::push_nl(int line_number, int current_pos) {
    this->tokens.push_back(
        Token(
            "NL",
            "\\n",
            {line_number+1, current_pos},
            {line_number+1, current_pos+1}
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

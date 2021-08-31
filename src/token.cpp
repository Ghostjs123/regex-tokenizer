#include <iostream>
#include <string>
#include <iomanip>
#include <tuple>
#include "token.h"
#include "util.h"

/**
 *  @brief Empty constructor. Creates an "undefined" Token.
**/
Token::Token() {
    this->type = "unknown";
    this->value = "undefined";
    this->line_start = -1;
    this->column_start = -1;
    this->line_end = -1;
    this->column_end = -1;
}
/**
 *  @brief char constructor.
**/
Token::Token(
    const std::string& type,
    char value,
    std::tuple<int, int> start,
    std::tuple<int, int> end
) {
    this->type = type;
    this->value = std::string(1, value);
    this->line_start = std::get<0>(start);
    this->column_start = std::get<1>(start);
    this->line_end = std::get<0>(end);
    this->column_end = std::get<1>(end);
}
/**
 *  @brief std::string constructor.
**/
Token::Token(
    const std::string& type,
    const std::string& value,
    std::tuple<int, int> start,
    std::tuple<int, int> end
) {
    this->type = type;
    this->value = value;
    this->line_start = std::get<0>(start);
    this->column_start = std::get<1>(start);
    this->line_end = std::get<0>(end);
    this->column_end = std::get<1>(end);
}

/**
 *  @brief Determines if single or double quotes can wrap the given Token value.
 *  @returns Returns single or double quotation character.
**/
char Token::get_quotes() const {
    if (this->value.size() > 0 && this->value[0] == '\'') {
        return '"';
    }

    return '\'';
}

/**
 *  @brief Returns the state of this Token as a std::string.
 *  @returns Token state information as a std::string.
**/
std::string Token::as_string() {
    char quote_char = this->get_quotes();
    std::string pos =
        std::to_string(line_start) + "," +
        std::to_string(column_start) + "-" +
        std::to_string(line_end) + "," +
        std::to_string(column_end) + ":";

    return pos + "\t" + type + "\t" + quote_char + value + quote_char; 
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
    char quote_char = token.get_quotes();
    std::string pos =
        std::to_string(token.line_start) + "," +
        std::to_string(token.column_start) + "-" +
        std::to_string(token.line_end) + "," +
        std::to_string(token.column_end) + ":";
    std::string val = quote_char + token.value + quote_char;

    os << std::left << std::setw(20) << pos
       << std::left << std::setw(15) << token.type 
       << std::left << std::setw(15) << val;

    return os;
}

bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.type == rhs.type && lhs.value == rhs.value;
}

bool operator!=(const Token& lhs, const Token& rhs) {
    return !(lhs == rhs);
}

Token::operator std::string() {
    char quote_char = this->get_quotes();
    return 
        std::to_string(this->line_start) + "," +
        std::to_string(this->column_start) + "-" +
        std::to_string(this->line_end) + "," +
        std::to_string(this->column_end) + ": " +
        this->type + " " + quote_char + this->value + quote_char;
}

#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <tuple>

class Token {
	public:
		std::string type, value;
		int line_start, line_end, column_start, column_end;
		
		Token();
        Token(
            const std::string& type,
            char value,
            std::tuple<int, int> start,
            std::tuple<int, int> end
        );
		Token(
            const std::string& type,
            const std::string& value,
            std::tuple<int, int> start,
            std::tuple<int, int> end
        );

        char get_quotes() const;
		std::string as_string();
        friend std::ostream& operator<<(std::ostream& os, const Token& token);
		friend bool operator==(const Token& lhs, const Token& rhs);
		friend bool operator!=(const Token& lhs, const Token& rhs);

    	operator std::string();
};

#endif

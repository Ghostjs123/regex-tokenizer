#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <tuple>

int main() {
    std::vector<std::tuple<std::string, std::regex>> regexs = {
        {"OP", std::regex("^(\\()")},
        {"OP", std::regex("^(\\))")},
        {"NAME", std::regex("^([a-zA-Z0-9_]*)")}
    };

    std::string str = ")asd";

    while (str.size() > 0) {
        std::smatch match;

        for (auto regex_tuple : regexs) {
            if (std::regex_search(str, match, std::get<1>(regex_tuple))) {
                std::cout << match[0] << std::endl;
                str.erase(0, ((std::string)match[0]).size());
            }
        }
    }

    return 0;
}

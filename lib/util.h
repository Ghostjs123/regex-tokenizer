#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>

bool is_number(const std::string& s);

std::string pad_string(std::string str, int size, char c=' ');

// template <class T>
// bool vector_contains(vector<T> vec, T x);

std::string ltrim(std::string s);

std::string read_file(std::string fname);
std::vector<std::string> read_lines(std::string fname);
bool file_exists(std::string fname);

#endif

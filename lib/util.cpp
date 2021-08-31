#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <vector>
#include <string>

bool is_number(const std::string& s) {
    try {
        (void)std::stod(s);
        return true;
    }
    catch(...) {
        return false;
    }
}

std::string pad_string(std::string str, int size, char c=' ') {
    if ((int)str.size() > size) {
        std::string msg = "attempted to pad \'" 
             + str + "\' of length " + std::to_string(str.size()) 
             + " to a length of " + std::to_string(size);
        throw std::runtime_error(msg);
    }
    str.insert(0, size - str.size(), c);
    return str;
}

// template <class T>
// bool vector_contains(vector<T> vec, T x) {
//     return find(vec.begin(), vec.end(), x) != vec.end();
// }

std::string ltrim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) { return !isspace(c); }));
    return s;
}

std::string read_file(std::string fname) {
    std::ifstream ifs(fname);
    std::string contents( (std::istreambuf_iterator<char>(ifs) ),
                     (std::istreambuf_iterator<char>()    ) );
    return contents;
}

std::vector<std::string> read_lines(std::string fname) {
    std::vector<std::string> results;

    std::fstream file;
    file.open(fname, std::ios::in);
    if (file.is_open()) {
        std::string tp;
        while(std::getline(file, tp)) {
            results.push_back(tp);
        }
    }

    return results;
}

bool file_exists(std::string fname) {
    struct stat buffer;
    return (stat (fname.c_str(), &buffer) == 0);
}

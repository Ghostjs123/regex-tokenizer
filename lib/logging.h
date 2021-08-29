#ifndef LOGGING_H
#define LOGGING_H

// singleton example found here
// https://refactoring.guru/design-patterns/singleton/cpp/example#example-0

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define DEBUG   3
#define INFO    2
#define WARNING 1

class Logger {
private:
    int mode;
    std::string fname;
    std::ofstream f;
    std::string indent;

    std::string get_mode_string();
protected:
    Logger();
    Logger(std::string fname);

    static Logger* logger;
public:
    ~Logger();

    // not cloneable
    Logger(Logger &other) = delete;
    // not assignable
    void operator=(const Logger&) = delete;

    static Logger* get_instance();
    static Logger* get_instance(const std::string& value);


    void log(std::string msg, int mode);
    void set_mode(int mode);
    void add_indent(int amt);
    void sub_indent(int amt);
    void close();
};

#endif

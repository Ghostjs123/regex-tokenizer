#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "logging.h"

// #define DEBUG   3
// #define INFO    2
// #define WARNING 1

using std::cout;
using std::endl;

Logger* Logger::logger = nullptr;

Logger::Logger() {
    cout << "Logger set to '" << "output_log" << "'" << endl;
    this->f.open("output_log", std::fstream::out | std::fstream::trunc); 
    this->mode = WARNING;
    this->indent = "";
}

Logger::Logger(std::string fname) {
    cout << "Logger set to '" << fname << "'" << endl;
    this->f.open(fname, std::fstream::out | std::fstream::trunc);
    this->mode = WARNING;
    this->indent = "";
}

Logger* Logger::get_instance() {
    if(logger == nullptr){
        logger = new Logger();
    }
    return logger;
}

Logger* Logger::get_instance(const std::string& fname) {
    if(logger == nullptr){
        logger = new Logger(fname);
    }
    return logger;
}

Logger::~Logger() {
    this->close();
}

std::string Logger::get_mode_string() {
    if (this->mode == DEBUG) return "DEBUG";
    if (this->mode == INFO) return "INFO";
    if (this->mode == WARNING) return "WARNING";
    throw std::runtime_error("logger - unknown mode: " + this->mode);
}

void Logger::log(std::string msg, int mode) {
    if (mode <= this-> mode) {
        cout << this->indent << msg << endl;
        if (this->f.is_open()) this->f << this->indent << msg << '\n';
    }
}
    
void Logger::set_mode(int mode) {
    if (mode == 1 || mode == 2 || mode == 3) this->mode = mode;
}

void Logger::add_indent(int amt) {
    for (int i=0; i < amt; i++) {
        this->indent += " ";
    }
}

void Logger::sub_indent(int amt) {
    if (indent.size() < amt) {
        throw std::runtime_error("Attempted to subtract '" + std::to_string(amt) 
                + "' from indent of size '" + std::to_string(this->indent.size()) + "'");
    }
    this->indent = this->indent.substr(0, this->indent.size()-amt);
}

void Logger::close() {
    if (this->f.is_open()) {
        this->f.close();
    }
}

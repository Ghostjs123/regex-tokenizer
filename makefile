includes = -Ilib -Isrc -Iunit_tests
default_args = -pedantic -g

libs = util.o logging.o unit-testing-util.o
tokenizer = regex-tokenizer.o token.o -lncurses $(libs)

regex-tokenizer-main: regex-tokenizer-main.cpp $(tokenizer)
	g++ regex-tokenizer-main.cpp $(tokenizer) $(default_args) $(includes) -o regex-tokenizer-main

# one off test files
test_regex: test_regex.cpp
	g++ test_regex.cpp $(default_args) -o test_regex

# ===============================================================================
# Object Files

# src/

regex-tokenizer.o: src/regex-tokenizer.cpp src/regex-tokenizer.h
	g++ src/regex-tokenizer.cpp $(includes) $(default_args) -c -o regex-tokenizer.o

token.o: src/token.cpp src/token.h
	g++ src/token.cpp $(includes) $(default_args) -c -o token.o

# lib/

util.o: lib/util.cpp lib/util.h
	g++ lib/util.cpp $(includes) $(default_args) -c -o util.o

logging.o: lib/logging.cpp lib/logging.h
	g++ lib/logging.cpp $(includes) $(default_args) -c -o logging.o

# unit_tests/

unit-testing-util.o: unit_tests/unit-testing-util.cpp
	g++ unit_tests/unit-testing-util.cpp $(includes) $(default_args) -c -o unit-testing-util.o

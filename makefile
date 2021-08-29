includes = -Ilib -Isrc
default_args = -pedantic -g

libs = util.o logging.o
tokenizer = regex-tokenizer.o token.o -lncurses

regex-tokenizer-main: regex-tokenizer-main.cpp
	g++ regex-tokenizer-main.cpp $(tokenizer) $(libs) $(default_args) $(includes) -o regex-tokenizer-main

# ===============================================================================
# Object Files

regex-tokenizer.o: src/regex-tokenizer.cpp src/regex-tokenizer.h
	g++ src/regex-tokenizer.cpp $(includes) $(default_args) -c -o regex-tokenizer.o

token.o: src/token.cpp src/token.h
	g++ src/token.cpp $(includes) $(default_args) -c -o token.o

# lib/

util.o: lib/util.cpp lib/util.h
	g++ lib/util.cpp $(includes) $(default_args) -c -o util.o

logging.o: lib/logging.cpp lib/logging.h
	g++ lib/logging.cpp $(includes) $(default_args) -c -o logging.o

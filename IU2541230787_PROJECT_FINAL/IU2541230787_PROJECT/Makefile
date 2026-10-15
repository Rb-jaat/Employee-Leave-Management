# Employee Leave Management - build file:  make | make test | make unit | make evidence | make clean
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Wold-style-cast -Wshadow -Werror -Iinclude
SRCS     := $(filter-out src/main.cpp,$(wildcard src/*.cpp))

elm: src/main.cpp $(SRCS) $(wildcard include/elm/*.h)
	$(CXX) $(CXXFLAGS) -o $@ src/main.cpp $(SRCS)

unit: $(SRCS) Tests/unit_tests.cpp
	$(CXX) $(CXXFLAGS) -g -fsanitize=address,undefined -o Tests/unit_tests Tests/unit_tests.cpp $(SRCS)
	./Tests/unit_tests

test: elm
	python3 Tests/run_tests.py

clean:
	rm -rf elm Tests/unit_tests Tests/_work

# source files under test in this dir
TESTSOURCE=../../quote.cpp
TESTOBJECT=quote.o

CXX      ?= g++
CXXFLAGS ?= -Wall -gdwarf-4
LDFLAGS  ?= -Wall -gdwarf-4

CXXFLAGS += -I `node -e "require('catch')"`

MAIN = main.cpp

all: pin

PINSOURCES = pin.cpp $(MAIN) 
PINOBJECTS = $(PINSOURCES:.cpp=.o) $(TESTOBJECT)

$(TESTOBJECT): $(TESTSOURCE)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

pin: $(PINOBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(PINOBJECTS)

.PHONY: all debug clean

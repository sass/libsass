# source files under test in this dir
TESTOBJECT=../../lib/libsass.a

CXX      ?= g++
CXXFLAGS ?= -Wall -gdwarf-4
LDFLAGS  ?= -Wall -gdwarf-4

CXXFLAGS += -std=c++0x
LDFLAGS  += -std=c++0x

CXXFLAGS += -I `node -e "require('catch')"`

MAIN = main.cpp

all: pin test-all

PINSOURCES = next_unescaped_interpolant.cpp $(MAIN) 
PINOBJECTS = $(PINSOURCES:.cpp=.o) $(TESTOBJECT)

ALLSOURCES = $(PINSOURCES)

test: $(TESTOBJECT)

$(TESTOBJECT): 
	$(MAKE) -C ../.. static

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

pin: $(PINOBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(PINOBJECTS)

test-all: $(TESTOBJECT)
	$(CXX) $(LDFLAGS) -o $@ $(PINOBJECTS)

.PHONY: all debug clean test

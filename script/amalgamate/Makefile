CXX ?= c++

CXXFLAGS := -std=c++11
CXXFLAGS_OPT := $(CXXFLAGS) -O2 -s
CXXFLAGS_DBG := $(CXXFLAGS) -fsanitize=address -g -O1 -fno-omit-frame-pointer
CXXFLAGS_FASTBUILD := $(CXXFLAGS) -O0 -s

build/amalgamate: amalgamate.cpp | build
	$(CXX) $(CXXFLAGS_FASTBUILD) -o build/amalgamate amalgamate.cpp

build:
	@mkdir build

clean: | build
	rm -rf build

.PHONY: amalgamate clean

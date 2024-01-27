CXX ?= c++

CXXFLAGS := -I ../include/ -g -O1 -fno-omit-frame-pointer

LIBSASS_CPPSTD ?= c++11
CXXFLAGS += -std=$(LIBSASS_CPPSTD)
LDFLAGS  += -std=$(LIBSASS_CPPSTD)

test: test_shared_ptr test_util_string

test_shared_ptr: build/test_shared_ptr
	build/test_shared_ptr

test_ordered_map: build/test_ordered_map
	build/test_ordered_map

test_character: build/test_character
	build/test_character

test_string_utils: build/test_string_utils
	build/test_string_utils

test_source_data: build/test_source_data
	build/test_source_data

test_lurlparser: build/test_lurlparser
	build/test_lurlparser

test_offset: build/test_offset
	build/test_offset

test_units: build/test_units
	build/test_units

build:
	@mkdir build

build/test_shared_ptr: test_shared_ptr.cpp ../src/memory/shared_ptr.cpp | build
	$(CXX) $(CXXFLAGS) ../src/memory/allocator.cpp ../src/memory/shared_ptr.cpp -o build/test_shared_ptr test_shared_ptr.cpp

build/test_ordered_map: test_ordered_map.cpp ../src/memory/shared_ptr.cpp | build
	$(CXX) $(CXXFLAGS) ../src/memory/allocator.cpp -o build/test_ordered_map test_ordered_map.cpp

build/test_character: test_character.cpp ../src/character.cpp | build
	$(CXX) $(CXXFLAGS) ../src/memory/allocator.cpp ../src/character.cpp -o build/test_character test_character.cpp

build/test_string_utils: test_string_utils.cpp ../src/string_utils.cpp | build
	$(CXX) $(CXXFLAGS) ../src/memory/allocator.cpp ../src/character.cpp ../src/string_utils.cpp -o build/test_string_utils test_string_utils.cpp

build/test_source_data: test_source_data.cpp ../src/source.cpp ../src/offset.cpp ../src/position.cpp | build
	$(CXX) $(CXXFLAGS) ../src/memory/allocator.cpp ../src/randomize.cpp -o build/test_source_data test_source_data.cpp ../src/source.cpp ../src/source_span.cpp ../src/source_state.cpp ../src/offset.cpp ../src/position.cpp

build/test_lurlparser: test_lurlparser.cpp ../src/LUrlParser/LUrlParser.cpp | build
	$(CXX) $(CXXFLAGS) -o build/test_lurlparser test_lurlparser.cpp ../src/LUrlParser/LUrlParser.cpp

build/test_offset: test_offset.cpp ../src/offset.cpp | build
	$(CXX) $(CXXFLAGS) ../src/memory/allocator.cpp -o build/test_offset test_offset.cpp ../src/offset.cpp

build/test_units: test_units.cpp ../src/units.cpp | build
	$(CXX) $(CXXFLAGS) ../src/memory/allocator.cpp -o build/test_units test_units.cpp ../src/units.cpp

clean: | build
	rm -rf build

.PHONY: test test_shared_ptr test_ordered_map test_string_utils test_source_data test_lurlparser test_offset clean

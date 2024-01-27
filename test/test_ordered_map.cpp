#include "../src/ordered_map.hpp"
#include "assert.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

bool TestOrderedMap() {
  Sass::ordered_map<std::string, double> map;
  ASSERT_NR_EQ(map.size(), 0);
  map.push_back("first", 42);
  // Test size getter
  ASSERT_NR_EQ(map.size(), 1);
  // Test front/back accessor
  ASSERT_NR_EQ(map.front().first, "first");
  ASSERT_NR_EQ(map.front().second, 42);
  ASSERT_NR_EQ(map.back().first, "first");
  ASSERT_NR_EQ(map.back().second, 42);
  // Test array accessor
  map["first"] = 1; // change
  map["second"] = 2; // append
  map["third"] = 3; // append
  // Test count getter
  ASSERT_NR_EQ(map.count("first"), 1);
  ASSERT_NR_EQ(map.count("seven"), 0);
  // Test array accessors
  ASSERT_NR_EQ(map[0].first, "first");
  ASSERT_NR_EQ(map[1].first, "second");
  ASSERT_NR_EQ(map[2].first, "third");
  ASSERT_NR_EQ(map["first"], 1);
  ASSERT_NR_EQ(map["second"], 2);
  ASSERT_NR_EQ(map["third"], 3);
  // Test size getter
  ASSERT_NR_EQ(map.size(), 3);
  // Test front/back accessor
  ASSERT_NR_EQ(map.front().first, "first");
  ASSERT_NR_EQ(map.front().second, 1);
  ASSERT_NR_EQ(map.back().first, "third");
  ASSERT_NR_EQ(map.back().second, 3);
  // Erase front item by key
  ASSERT_TRUE(map.erase("first"));
  // Test size getter
  ASSERT_NR_EQ(map.size(), 2);
  // Test front/back accessor
  ASSERT_NR_EQ(map.front().first, "second");
  ASSERT_NR_EQ(map.front().second, 2);
  // Erase front item by index
  ASSERT_TRUE(map.erase(0));
  // Test size getter
  ASSERT_NR_EQ(map.size(), 1);
  // Test front/back accessor
  ASSERT_NR_EQ(map.front().first, "third");
  ASSERT_NR_EQ(map.front().second, 3);
  // Search iterator by key
  auto it = map.find("third");
  // Test iterator accessor
  ASSERT_NR_EQ(it->first, "third");
  ASSERT_NR_EQ(it->second, 3);
  // Test iterator modifier
  it->second = 42;
  // Check again by fetching front
  ASSERT_NR_EQ(map[0].second, 42);
  ASSERT_NR_EQ(map.front().second, 42);
  // Erase front item by index
  ASSERT_TRUE(map.erase("third"));
  ASSERT_TRUE(map.empty());
  // All have passed
  return true;
}

int main(int argc, char **argv) {
  INIT_TEST_RESULTS;
  TEST(TestOrderedMap);
  REPORT_TEST_RESULTS;
}

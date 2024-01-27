#include "../src/position.hpp"
#include "assert.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace Sass;

bool testOffsetMove() {
  const char* text1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  ASSERT_STR_EQ(Offset::move(text1, Offset::init(0, 8)),
    "IJKLMNOPQRSTUVWXYZ");
  const char* text2 = "ABC\nDEFGHIJKLMNOPQRSTUVW\nXYZ";
  ASSERT_STR_EQ(Offset::move(text2, Offset::init(1, 5)),
    "IJKLMNOPQRSTUVW\nXYZ");
  ASSERT_STR_EQ(Offset::move(text2, Offset::init(1, 0)),
    "DEFGHIJKLMNOPQRSTUVW\nXYZ");
  ASSERT_STR_EQ(Offset::move(text2, Offset::init(2, 0)),
    "XYZ");
  ASSERT_STR_EQ(Offset::move(text2, Offset::init(2, 3)), "");
  ASSERT_NR_EQ(Offset::move(text2, Offset::init(3, 5)), 0);
  ASSERT_NR_EQ(Offset::move(text2, Offset::init(2, 4)), 0);
  ASSERT_NR_EQ(Offset::move(text2, Offset::init(1, 20)), 0);
  return true;
}

int main(int argc, char **argv) {
  INIT_TEST_RESULTS;
  TEST(testOffsetMove);
  REPORT_TEST_RESULTS;
}

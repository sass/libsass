#include "../src/source.hpp"
#include "assert.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

using namespace Sass;

bool testSourceFileBasic() {
  const char* txt =
    "Line A\n"
    "Line B\n"
    "Line C";
  SourceFile source("sass://", txt, -1);
  ASSERT_NR_EQ(source.countLines(), 3);
  ASSERT_STR_EQ(source.getLine(0), "Line A");
  ASSERT_STR_EQ(source.getLine(1), "Line B");
  ASSERT_STR_EQ(source.getLine(2), "Line C");
  return true;
}

bool testSourceFileCrLf() {
  const char* txt =
    "Line A\r\n"
    "Line B\r\n"
    "Line C\r";
  SourceFile source("sass://", txt, -1);
  ASSERT_NR_EQ(source.countLines(), 3);
  ASSERT_STR_EQ(source.getLine(0), "Line A");
  ASSERT_STR_EQ(source.getLine(1), "Line B");
  ASSERT_STR_EQ(source.getLine(2), "Line C");
  return true;
}

bool testSourceFileEmpty() {
  const char* txt =
    "\n"
    "\n"
    "";
  SourceFile source("sass://", txt, -1);
  ASSERT_NR_EQ(source.countLines(), 3);
  ASSERT_STR_EQ(source.getLine(0), "");
  ASSERT_STR_EQ(source.getLine(1), "");
  ASSERT_STR_EQ(source.getLine(2), "");
  return true;
}

bool testSourceFileEmptyCrLf() {
  const char* txt =
    "\r\n"
    "\r\n"
    "\r";
  SourceFile source("sass://", txt, -1);
  ASSERT_NR_EQ(source.countLines(), 3);
  ASSERT_STR_EQ(source.getLine(0), "");
  ASSERT_STR_EQ(source.getLine(1), "");
  ASSERT_STR_EQ(source.getLine(2), "");
  return true;
}

bool testSourceFileEmptyTrail() {
  const char* txt =
    "\n"
    "\n"
    "\n";
  SourceFile source("sass://", txt, -1);
  ASSERT_NR_EQ(source.countLines(), 4);
  ASSERT_STR_EQ(source.getLine(0), "");
  ASSERT_STR_EQ(source.getLine(1), "");
  ASSERT_STR_EQ(source.getLine(2), "");
  ASSERT_STR_EQ(source.getLine(3), "");
  return true;
}

bool testSyntheticFileBasic() {
  const char* around =
    "Line A\n"
    "Line B\n"
    "Line C\n"
    "Line D\n"
    "Line E";
  SourceFileObj parent = SASS_MEMORY_NEW(
    SourceFile, "sass://", around, -1);
  SourceSpan pstate(parent,
    Offset::init(1, 4),
    Offset::init(0, 0));
  SyntheticFile source("[ADD]", parent, pstate);
  ASSERT_NR_EQ(source.countLines(), 5);
  ASSERT_STR_EQ(source.getLine(0), "Line A");
  ASSERT_STR_EQ(source.getLine(1), "Line[ADD] B");
  ASSERT_STR_EQ(source.getLine(2), "Line C");
  ASSERT_STR_EQ(source.getLine(3), "Line D");
  ASSERT_STR_EQ(source.getLine(4), "Line E");
  return true;
}

bool testSyntheticFileBasicMulti() {
  const char* around =
    "Line A\n"
    "Line B\n"
    "Line C\n"
    "Line D\n"
    "Line E";
  SourceFileObj parent = SASS_MEMORY_NEW(
    SourceFile, "sass://", around, -1);
  SourceSpan pstate(parent,
    Offset::init(1, 1),
    Offset::init(1, 2));
  SyntheticFile source("[ADD]", parent, pstate);
  ASSERT_NR_EQ(source.countLines(), 4);
  ASSERT_STR_EQ(source.getLine(0), "Line A");
  ASSERT_STR_EQ(source.getLine(1), "L[ADD]ne C");
  ASSERT_STR_EQ(source.getLine(2), "Line D");
  ASSERT_STR_EQ(source.getLine(3), "Line E");
  return true;
}

bool testSyntheticFileMulti() {
  const char* around =
    "Line A\n"
    "Line B\n"
    "Line C\n"
    "Line D\n"
    "Line E";
  SourceFileObj parent = SASS_MEMORY_NEW(
    SourceFile, "sass://", around, -1);
  SourceSpan pstate(parent,
    Offset::init(1, 4),
    Offset::init(0, 0));
  SyntheticFile source("[ADD]\n[ANOTHER]\n[MORE]", parent, pstate);
  ASSERT_NR_EQ(source.countLines(), 7);
  ASSERT_STR_EQ(source.getLine(0), "Line A");
  ASSERT_STR_EQ(source.getLine(1), "Line[ADD]");
  ASSERT_STR_EQ(source.getLine(2), "[ANOTHER]");
  ASSERT_STR_EQ(source.getLine(3), "[MORE] B");
  ASSERT_STR_EQ(source.getLine(4), "Line C");
  ASSERT_STR_EQ(source.getLine(5), "Line D");
  ASSERT_STR_EQ(source.getLine(6), "Line E");
  return true;
}

bool testSyntheticFileMultiMulti() {
  const char* around =
    "Line A\n"
    "Line B\n"
    "Line C\n"
    "Line D\n"
    "Line E";
  SourceFileObj parent = SASS_MEMORY_NEW(
    SourceFile, "sass://", around, -1);
  SourceSpan pstate(parent,
    Offset::init(1, 4),
    Offset::init(1, 5));
  SyntheticFile source("[ADD]\n[ANOTHER]\n[MORE]", parent, pstate);
  ASSERT_NR_EQ(source.countLines(), 6);
  ASSERT_STR_EQ(source.getLine(0), "Line A");
  ASSERT_STR_EQ(source.getLine(1), "Line[ADD]");
  ASSERT_STR_EQ(source.getLine(2), "[ANOTHER]");
  ASSERT_STR_EQ(source.getLine(3), "[MORE]C");
  ASSERT_STR_EQ(source.getLine(4), "Line D");
  ASSERT_STR_EQ(source.getLine(5), "Line E");
  return true;
}

bool testSourceFileUnicode() {
  const char* txt =
    "Line A\n"
    "[a=b ï]\n"
    "Line C";
  SourceFile source("sass://", txt, -1);
  ASSERT_NR_EQ(source.countLines(), 3);
  ASSERT_STR_EQ(source.getLine(0), "Line A");
  ASSERT_STR_EQ(source.getLine(1), "[a=b ï]");
  ASSERT_STR_EQ(source.getLine(2), "Line C");
  return true;
}

bool testSyntheticFileUnicode1() {
  const char* around =
    "Line A\n"
    "[ä=ö ï]\n"
    "Line C";
  SourceFileObj parent = SASS_MEMORY_NEW(
    SourceFile, "sass://", around, -1);
  SourceSpan pstate(parent,
    Offset::init(1, 1),
    Offset::init(0, 4));
  SyntheticFile source("[ADD]", parent, pstate);
  ASSERT_NR_EQ(source.countLines(), 3);
  ASSERT_STR_EQ(source.getLine(0), "Line A");
  ASSERT_STR_EQ(source.getLine(1), "[[ADD]ï]");
  ASSERT_STR_EQ(source.getLine(2), "Line C");
  return true;
}

bool testSyntheticFileUnicode2() {
  const char* around =
    "Line A\n"
    "[ä=ö ï]\n"
    "Line C";
  SourceFileObj parent = SASS_MEMORY_NEW(
    SourceFile, "sass://", around, -1);
  SourceSpan pstate(parent,
    Offset::init(1, 2),
    Offset::init(0, 3));
  SyntheticFile source("[ADD]", parent, pstate);
  ASSERT_NR_EQ(source.countLines(), 3);
  ASSERT_STR_EQ(source.getLine(0), "Line A");
  ASSERT_STR_EQ(source.getLine(1), "[ä[ADD]ï]");
  ASSERT_STR_EQ(source.getLine(2), "Line C");
  return true;
}

}  // namespace

int main(int argc, char **argv) {
  INIT_TEST_RESULTS;
  TEST(testSourceFileBasic);
  TEST(testSourceFileCrLf);
  TEST(testSourceFileEmpty);
  TEST(testSourceFileEmptyCrLf);
  TEST(testSourceFileEmptyTrail);
  TEST(testSourceFileUnicode);
  TEST(testSyntheticFileBasic);
  TEST(testSyntheticFileMulti);
  TEST(testSyntheticFileBasicMulti);
  TEST(testSyntheticFileMultiMulti);
  TEST(testSyntheticFileUnicode1);
  TEST(testSyntheticFileUnicode2);
  REPORT_TEST_RESULTS;
}

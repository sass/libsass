#include "../src/memory/shared_ptr.hpp"
#include "../src/LUrlParser/LUrlParser.hpp"
#include "assert.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

bool TestLUrlParser() {
  using LUrlParser::clParseURL;
  clParseURL URL = clParseURL::ParseURL(
    "https://John:Dow@github.com:443/corporateshark/LUrlParser?query#hash");
  if (URL.IsValid())
  {
    ASSERT_STR_EQ(URL.m_Scheme, "https");
    ASSERT_STR_EQ(URL.m_Host, "github.com");
    ASSERT_STR_EQ(URL.m_Port, "443");
    ASSERT_STR_EQ(URL.m_Path, "corporateshark/LUrlParser");
    ASSERT_STR_EQ(URL.m_Query, "query");
    ASSERT_STR_EQ(URL.m_Fragment, "hash");
    ASSERT_STR_EQ(URL.m_UserName, "John");
    ASSERT_STR_EQ(URL.m_Password, "Dow");
  }
  return true;
}

int main(int argc, char **argv) {
  INIT_TEST_RESULTS;
  TEST(TestLUrlParser);
  REPORT_TEST_RESULTS;
}

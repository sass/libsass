#include <string>

namespace Sass {
  using namespace std;

  string unquote(const string& s)
  {
    if (s.empty()) return "";
    if (s.length() == 1) {
      if (s[0] == '"' || s[0] == '\'') return "";
    }
    char q;
    if      (*s.begin() == '"'  && *s.rbegin() == '"')  q = '"';
    else if (*s.begin() == '\'' && *s.rbegin() == '\'') q = '\'';
    else                                                return s;
    string t;
    t.reserve(s.length()-2);
    for (size_t i = 1, L = s.length()-1; i < L; ++i) {
      // if we see a quote, we need to remove the preceding backslash from t
      if (s[i-1] == '\\' && s[i] == q) t.erase(t.length()-1);
      t.push_back(s[i]);
    }
    return t;
  }

  string quote(const string& s, char q)
  {
    if (s.empty()) return string(2, q);
    if (!q || s[0] == '"' || s[0] == '\'') return s;
    string t;
    t.reserve(s.length()+2);
    t.push_back(q);
    for (size_t i = 0, L = s.length(); i < L; ++i) {
      if (s[i] == q) t.push_back('\\');
      t.push_back(s[i]);
    }
    t.push_back(q);
    return t;
  }

}

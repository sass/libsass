#include "ast.hpp"
#include "util.hpp"
#include "utf8/checked.h"

namespace Sass {

  // double escape every escape sequences
  // escape unescaped quotes and backslashes
  string string_escape(const string& str)
  {
    string out("");
    for (auto i : str) {
      // escape some characters
      if (i == '"') out += '\\';
      if (i == '\'') out += '\\';
      if (i == '\\') out += '\\';
      out += i;
    }
    return out;
  }

  // unescape every escape sequence
  // only removes unescaped backslashes
  string string_unescape(const string& str)
  {
    string out("");
    bool esc = false;
    for (auto i : str) {
      if (esc || i != '\\') {
        esc = false;
        out += i;
      } else {
        esc = true;
      }
    }
    // open escape sequence at end
    // maybe it should thow an error
    if (esc) { out += '\\'; }
    return out;
  }

  // evacuate unescaped quoted
  // leave everything else untouched
  string evacuate_quotes(const string& str)
  {
    string out("");
    bool esc = false;
    for (auto i : str) {
      if (!esc) {
        // ignore next character
        if (i == '\\') esc = true;
        // evacuate unescaped quotes
        else if (i == '"') out += '\\';
        else if (i == '\'') out += '\\';
      }
      // get escaped char now
      else { esc = false; }
      // remove nothing
      out += i;
    }
    return out;
  }

  // double escape all escape sequences
  // keep unescaped quotes and backslashes
  string evacuate_escapes(const string& str)
  {
    string out("");
    bool esc = false;
    for (auto i : str) {
      if (i == '\\' && !esc) {
        out += '\\';
        out += '\\';
        esc = true;
      } else if (esc && i == '"') {
        out += '\\';
        out += i;
        esc = false;
      } else if (esc && i == '\'') {
        out += '\\';
        out += i;
        esc = false;
      } else if (esc && i == '\\') {
        out += '\\';
        out += i;
        esc = false;
      } else {
        esc = false;
        out += i;
      }
    }
    if (esc) out += 'Z';
    return out;
  }

  // bell character is replaces with space
  string string_to_output(const string& str)
  {
    string out("");
    for (auto i : str) {
      if (i == 10) {
        out += ' ';
      } else {
        out += i;
      }
    }
    return out;
  }

  namespace Util {
    using std::string;

    string normalize_underscores(const string& str) {
      string normalized = str;
      for(size_t i = 0, L = normalized.length(); i < L; ++i) {
        if(normalized[i] == '_') {
          normalized[i] = '-';
        }
      }
      return normalized;
    }

    string normalize_decimals(const string& str) {
      string prefix = "0";
      string normalized = str;

      return normalized[0] == '.' ? normalized.insert(0, prefix) : normalized;
    }

    // compress a color sixtuplet if possible
    // input: "#CC9900" -> output: "#C90"
    string normalize_sixtuplet(const string& col) {
      if(
        col.substr(1, 1) == col.substr(2, 1) &&
        col.substr(3, 1) == col.substr(4, 1) &&
        col.substr(5, 1) == col.substr(6, 1)
      ) {
        return string("#" + col.substr(1, 1)
                          + col.substr(3, 1)
                          + col.substr(5, 1));
      } else {
        return string(col);
      }
    }

    bool isPrintable(Ruleset* r) {
      if (r == NULL) {
        return false;
      }

      Block* b = r->block();

      bool hasSelectors = static_cast<Selector_List*>(r->selector())->length() > 0;

      if (!hasSelectors) {
        return false;
      }

      bool hasDeclarations = false;
      bool hasPrintableChildBlocks = false;
      for (size_t i = 0, L = b->length(); i < L; ++i) {
        Statement* stm = (*b)[i];
        if (dynamic_cast<Has_Block*>(stm)) {
          Block* pChildBlock = ((Has_Block*)stm)->block();
          if (isPrintable(pChildBlock)) {
            hasPrintableChildBlocks = true;
          }
        } else {
          hasDeclarations = true;
        }

        if (hasDeclarations || hasPrintableChildBlocks) {
          return true;
        }
      }

      return false;
    }

    bool isPrintable(Feature_Block* f) {
      if (f == NULL) {
        return false;
      }

      Block* b = f->block();

      bool hasSelectors = f->selector() && static_cast<Selector_List*>(f->selector())->length() > 0;

      bool hasDeclarations = false;
      bool hasPrintableChildBlocks = false;
      for (size_t i = 0, L = b->length(); i < L; ++i) {
        Statement* stm = (*b)[i];
        if (!stm->is_hoistable() && f->selector() != NULL && !hasSelectors) {
          // If a statement isn't hoistable, the selectors apply to it. If there are no selectors (a selector list of length 0),
          // then those statements aren't considered printable. That means there was a placeholder that was removed. If the selector
          // is NULL, then that means there was never a wrapping selector and it is printable (think of a top level media block with
          // a declaration in it).
        }
        else if (typeid(*stm) == typeid(Declaration) || typeid(*stm) == typeid(At_Rule)) {
          hasDeclarations = true;
        }
        else if (dynamic_cast<Has_Block*>(stm)) {
          Block* pChildBlock = ((Has_Block*)stm)->block();
          if (isPrintable(pChildBlock)) {
            hasPrintableChildBlocks = true;
          }
        }

        if (hasDeclarations || hasPrintableChildBlocks) {
          return true;
        }
      }

      return false;
    }

    bool isPrintable(Media_Block* m) {
      if (m == NULL) {
        return false;
      }

      Block* b = m->block();

      bool hasSelectors = m->selector() && static_cast<Selector_List*>(m->selector())->length() > 0;

      bool hasDeclarations = false;
      bool hasPrintableChildBlocks = false;
      for (size_t i = 0, L = b->length(); i < L; ++i) {
        Statement* stm = (*b)[i];
        if (!stm->is_hoistable() && m->selector() != NULL && !hasSelectors) {
          // If a statement isn't hoistable, the selectors apply to it. If there are no selectors (a selector list of length 0),
          // then those statements aren't considered printable. That means there was a placeholder that was removed. If the selector
          // is NULL, then that means there was never a wrapping selector and it is printable (think of a top level media block with
          // a declaration in it).
        }
        else if (typeid(*stm) == typeid(Declaration) || typeid(*stm) == typeid(At_Rule)) {
          hasDeclarations = true;
        }
        else if (dynamic_cast<Has_Block*>(stm)) {
          Block* pChildBlock = ((Has_Block*)stm)->block();
          if (isPrintable(pChildBlock)) {
            hasPrintableChildBlocks = true;
          }
        }

        if (hasDeclarations || hasPrintableChildBlocks) {
          return true;
        }
      }

      return false;
    }

    bool isPrintable(Block* b) {
      if (b == NULL) {
        return false;
      }

      for (size_t i = 0, L = b->length(); i < L; ++i) {
        Statement* stm = (*b)[i];
        if (typeid(*stm) == typeid(Declaration) || typeid(*stm) == typeid(At_Rule)) {
          return true;
        }
        else if (typeid(*stm) == typeid(Ruleset)) {
          Ruleset* r = (Ruleset*) stm;
          if (isPrintable(r)) {
            return true;
          }
        }
        else if (typeid(*stm) == typeid(Feature_Block)) {
          Feature_Block* f = (Feature_Block*) stm;
          if (isPrintable(f)) {
            return true;
          }
        }
        else if (typeid(*stm) == typeid(Media_Block)) {
          Media_Block* m = (Media_Block*) stm;
          if (isPrintable(m)) {
            return true;
          }
        }
        else if (dynamic_cast<Has_Block*>(stm) && isPrintable(((Has_Block*)stm)->block())) {
          return true;
        }
      }

      return false;
    }

    string vecJoin(const vector<string>& vec, const string& sep)
    {
      switch (vec.size())
      {
        case 0:
            return string("");
        case 1:
            return vec[0];
        default:
            std::ostringstream os;
            os << vec[0];
            for (size_t i = 1; i < vec.size(); i++) {
              os << sep << vec[i];
            }
            return os.str();
      }
    }

     bool isAscii(int ch) {
         return ch >= 0 && ch < 128;
     }

  }
}

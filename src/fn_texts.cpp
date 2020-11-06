#include "fn_texts.hpp"

#include <iomanip>
#include "unicode.hpp"
#include "compiler.hpp"
#include "exceptions.hpp"
#include "ast_values.hpp"

#ifdef __MINGW32__
#include "windows.h"
#include "wincrypt.h"
#endif

namespace Sass {

  namespace Functions {

    namespace Texts {

      long _codepointForIndex(long index, long lengthInCodepoints, bool allowNegative = false) {
        if (index == 0) return 0;
        if (index > 0) return std::min(index - 1, lengthInCodepoints);
        long result = lengthInCodepoints + index;
        if (result < 0 && !allowNegative) return 0;
        return result;
      }

      BUILT_IN_FN(unquote)
      {
        String* string = arguments[0]->assertString(compiler, "string");
        if (!string->hasQuotes()) return string;
        sass::string copy(string->value());
        return SASS_MEMORY_NEW(String,
          string->pstate(), std::move(copy), false);
      }

      BUILT_IN_FN(quote)
      {
        if (Color* col = arguments[0]->isaColor()) {
          if (!col->disp().empty()) {
            sass::string copy(col->disp());
            return SASS_MEMORY_NEW(String,
              arguments[0]->pstate(), std::move(copy), true);
          }
        }
        String* string = arguments[0]->assertString(compiler, "string");
        if (string->hasQuotes()) return string;
        sass::string copy(string->value());
        return SASS_MEMORY_NEW(String,
          string->pstate(), std::move(copy), true);
      }

      BUILT_IN_FN(toUpperCase)
      {
        String* string = arguments[0]->assertString(compiler, "string");
        return SASS_MEMORY_NEW(String, pstate,
          StringUtils::toUpperCase(string->value()),
          string->hasQuotes());
      }

      BUILT_IN_FN(toLowerCase)
      {
        String* string = arguments[0]->assertString(compiler, "string");
        return SASS_MEMORY_NEW(String, pstate,
          StringUtils::toLowerCase(string->value()),
          string->hasQuotes());
      }

      BUILT_IN_FN(length)
      {
        String* string = arguments[0]->assertString(compiler, "string");
        size_t len = Unicode::codePointCount(string->value());
        return SASS_MEMORY_NEW(Number, pstate, (double)len);
      }

      BUILT_IN_FN(insert)
      {
        String* string = arguments[0]->assertString(compiler, "string");
        String* insert = arguments[1]->assertString(compiler, "insert");
        size_t len = Unicode::codePointCount(string->value());
        long index = arguments[2]->assertNumber(compiler, "index")
          ->assertUnitless(compiler, "index")
          ->assertInt(compiler, "index");

        // str-insert has unusual behavior for negative inputs. It guarantees that
        // the `$insert` string is at `$index` in the result, which means that we
        // want to insert before `$index` if it's positive and after if it's
        // negative.
        if (index < 0) {
          // +1 because negative indexes start counting from -1 rather than 0, and
          // another +1 because we want to insert *after* that index.
          index = (long)len + index + 2;
        }

        index = (long) _codepointForIndex(index, (long)len);

        sass::string str(string->value());
        str.insert(Unicode::byteOffsetAtPosition(
          str, index), insert->value());

        return SASS_MEMORY_NEW(String,
          pstate, std::move(str),
          string->hasQuotes());
      }

      BUILT_IN_FN(index)
      {
        String* string = arguments[0]->assertString(compiler, "string");
        String* substring = arguments[1]->assertString(compiler, "substring");

        sass::string str(string->value());
        sass::string substr(substring->value());

        size_t c_index = str.find(substr);
        if (c_index == sass::string::npos) {
          return SASS_MEMORY_NEW(Null, pstate);
        }

        return SASS_MEMORY_NEW(Number, pstate,
          (double)Unicode::codePointCount(str, c_index) + 1);
      }

      BUILT_IN_FN(slice)
      {
        String* string = arguments[0]->assertString(compiler, "string");
        Number* beg = arguments[1]->assertNumber(compiler, "start-at");
        Number* end = arguments[2]->assertNumber(compiler, "end-at");
        size_t len = Unicode::codePointCount(string->value());
        beg = beg->assertUnitless(compiler, "start");
        end = end->assertUnitless(compiler, "end");
        long begInt = beg->assertInt(compiler, "start");
        long endInt = end->assertInt(compiler, "end");

        // No matter what the start index is, an end
        // index of 0 will produce an empty string.
        if (endInt == 0) {
          return SASS_MEMORY_NEW(String,
            pstate, "", string->hasQuotes());
        }

        begInt = (long)_codepointForIndex(begInt, (long)len, false);
        endInt = (long)_codepointForIndex(endInt, (long)len, true);

        if (endInt == (long)len) endInt = (long)len - 1;
        if (endInt < begInt) {
          return SASS_MEMORY_NEW(String,
            pstate, "", string->hasQuotes());
        }

        const sass::string& value(string->value());
        sass::string::const_iterator begIt = value.begin();
        sass::string::const_iterator endIt = value.begin();
        utf8::advance(begIt, begInt + 0, value.end());
        utf8::advance(endIt, endInt + 1, value.end());

        return SASS_MEMORY_NEW(
          String, pstate,
          sass::string(begIt, endIt),
          string->hasQuotes());

      }

      BUILT_IN_FN(uniqueId)
      {
        sass::sstream ss; ss << "u"
          << std::setfill('0') << std::setw(8)
          << std::hex << getRandomUint32();
        return SASS_MEMORY_NEW(String,
          pstate, ss.str(), true);
      }

	    void registerFunctions(Compiler& ctx)
	    {

		    ctx.registerBuiltInFunction("unquote", "$string", unquote);
		    ctx.registerBuiltInFunction("quote", "$string", quote);
		    ctx.registerBuiltInFunction("to-upper-case", "$string", toUpperCase);
		    ctx.registerBuiltInFunction("to-lower-case", "$string", toLowerCase);
		    ctx.registerBuiltInFunction("str-length", "$string", length);
		    ctx.registerBuiltInFunction("str-insert", "$string, $insert, $index", insert);
		    ctx.registerBuiltInFunction("str-index", "$string, $substring", index);
		    ctx.registerBuiltInFunction("str-slice", "$string, $start-at, $end-at: -1", slice);
		    ctx.registerBuiltInFunction("unique-id", "", uniqueId);

	    }

    }


    void handle_utf8_error (const SourceSpan& pstate, BackTraces traces)
    {
      try {
       throw;
      }
      catch (utf8::invalid_code_point&) {
        traces.push_back(pstate);
        throw Exception::RuntimeException(traces,
          "utf8::invalid_code_point");
      }
      catch (utf8::not_enough_room&) {
        traces.push_back(pstate);
        throw Exception::RuntimeException(traces,
          "utf8::not_enough_room");
      }
      catch (utf8::invalid_utf8&) {
        traces.push_back(pstate);
        throw Exception::RuntimeException(traces,
          "utf8::invalid_utf8");
      }
      catch (...) { throw; }
    }

  }

}

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "file.hpp"
#include "terminal.hpp"

using namespace Sass;

extern "C" {

  void ADDCALL sass_chdir(const char* path)
  {
    if (path != nullptr) {
      CWD = File::rel2abs(path, CWD) + "/";
    }
  }

  void ADDCALL sass_print_stderr(const char* message)
  {
    Terminal::print(message, true);
  }

  void ADDCALL sass_print_stdout(const char* message)
  {
    Terminal::print(message, false);
  }

}

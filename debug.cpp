#include <stdio.h>
#include <sstream>

#include "debug.hpp"

namespace Sass {

  Log::Log() {} 

  std::ostringstream& Log::Get(TLogLevel level, void *p, const char *f, const char *filen, int lineno)
  {
	 os << "[LIBSASS] " << p << ":" << f << " " << filen << ":" << lineno << " ";
	 messageLevel = level;
	 return os;
  }
  std::ostringstream& Log::Get(TLogLevel level, const char *f, const char *filen, int lineno)
  {
	 os << "[LIBSASS] " << f << " " << filen << ":" << lineno << " ";
	 messageLevel = level;
	 return os;
  }
  Log::~Log()
  {
	 os << std::endl;
	 fprintf(stderr, "%s", os.str().c_str());
	 fflush(stderr);
  }
}

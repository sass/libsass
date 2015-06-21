#ifndef SASS_DEBUG_H
#define SASS_DEBUG_H

#include <stdint.h>

enum dbg_lvl_t : uint32_t {
	NONE = 0,
	TRIM = 1,
	CHUNKS = 2,
	SUBWEAVE = 4,
	WEAVE = 8,
	EXTEND_COMPOUND = 16,
	EXTEND_COMPLEX = 32,
	LCS = 64,
  EXTEND_OBJECT = 128,
	ALL = UINT32_MAX
};

namespace Sass {
  enum TLogLevel {logINFO, logTRACE};
  static TLogLevel LibsassLogReportingLevel = getenv("LIBSASS_TRACE") ? logTRACE : logINFO;
  class Log
  {
  public:
	Log();
	virtual ~Log();
	std::ostringstream& Get(TLogLevel level, void *p, const char *f, const char *filen, int lineno);
	std::ostringstream& Get(TLogLevel level, const char *f, const char *filen, int lineno);
  public:
  protected:
	std::ostringstream os;
  private:
	Log(const Log&);
	Log& operator =(const Log&);
  private:
	TLogLevel messageLevel;
  };
}

#define TRACE() \
  if (logTRACE > Sass::LibsassLogReportingLevel) ; \
  else Sass::Log().Get(Sass::logTRACE, __func__, __FILE__, __LINE__)

#define TRACEINST(obj) \
  if (logTRACE > Sass::LibsassLogReportingLevel) ; \
  else Sass::Log().Get(Sass::logTRACE, (obj), __func__, __FILE__, __LINE__)

#ifdef DEBUG

#ifndef DEBUG_LVL
const uint32_t debug_lvl = UINT32_MAX;
#else
const uint32_t debug_lvl = (DEBUG_LVL);
#endif // DEBUG_LVL

#define DEBUG_PRINT(lvl, x) if((lvl) & debug_lvl) { std::cerr << x; }
#define DEBUG_PRINTLN(lvl, x) if((lvl) & debug_lvl) { std::cerr << x << std::endl; }
#define DEBUG_EXEC(lvl, x) if((lvl) & debug_lvl) { x; }

#else // DEBUG

#define DEBUG_PRINT(lvl, x)
#define DEBUG_PRINTLN(lvl, x)
#define DEBUG_EXEC(lvl, x)

#endif // DEBUG

#endif // SASS_DEBUG

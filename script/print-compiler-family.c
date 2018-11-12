// Print the compiler family, used by ../Makefile.
#include <stdio.h>

int main()
{
  int code = printf(
#if defined(__clang__)
      "clang"
#elif defined(__GNUC__) || defined(__GNUG__)
      "gcc"
#else
      "other"
#endif
      "\n");
  if (code < 0)
  {
    fprintf(stderr, "error: printf returned %d\n", code);
    return 1;
  }
  return 0;
}

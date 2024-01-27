#include <random>

#ifdef __cplusplus
extern "C" {
#endif

void emscripten_bug() {
  std::random_device rd;
}

#ifdef __cplusplus
}
#endif

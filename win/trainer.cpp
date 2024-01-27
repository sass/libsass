#include <sass.h>
#include <stdio.h>
#include <windows.h>

uint64_t get_cpu_usage(bool aggregated) {

    FILETIME createTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;

    int rv = -1;
    if (aggregated) {
        rv = GetProcessTimes(GetCurrentProcess(),
            &createTime, &exitTime, &kernelTime, &userTime);
    }
    else {
        rv = GetThreadTimes(GetCurrentThread(),
            &createTime, &exitTime, &kernelTime, &userTime);
    }

    if (rv != -1)
    {
        ULONGLONG tcpu = userTime.dwHighDateTime;
        tcpu += kernelTime.dwHighDateTime;
        ULONGLONG usec = (tcpu << 32)
            + kernelTime.dwLowDateTime
            + userTime.dwLowDateTime;
        return (usec) / 10000;
    }

    return 0;
}

int main(int argc, const char* argv[])
{

  uint64_t start = get_cpu_usage(true);
  // get the input file from first argument or use default
  const char* input = argc > 1 ? argv[1] : "input.scss";
    // create compiler object holding config and states
  struct SassCompiler* compiler = sass_make_compiler();
  // add our current path to be searchable for import
  sass_compiler_add_include_paths(compiler, ".");
  // create the file context and get all related structs
  struct SassImport* import = sass_make_file_import(input);
  // each compiler must have exactly one entry point
  sass_compiler_set_entry_point(compiler, import);
  // entry point now passed to compiler, so its reference count was increased
  // in order to not leak memory we must release our own usage (usage after is UB)
  sass_delete_import(import); // decrease ref-count

  // context is set up, call the compile step now
  int status = sass_compiler_execute(compiler, true);
  // release allocated memory
  sass_delete_compiler(compiler);

  uint64_t delta = get_cpu_usage(true) - start;
  fprintf(stderr, "Took %.5fs\n", delta / 1000.0);

  // exit status
  return status;

}

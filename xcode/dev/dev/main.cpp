#include <stdio.h>
#include "../../../sass_context.h"

int main( int argc, const char* argv[] ) {
  
  // Get the input file from the first argument or use default
  const char* input = argc > 1 ? argv[1] : "styles.scss";
  
  // Create the file context and get all related structs
  struct Sass_File_Context* file_ctx = sass_make_file_context(input);
  struct Sass_Context* ctx = sass_file_context_get_context(file_ctx);
  struct Sass_Options* ctx_opt = sass_context_get_options(ctx);
  
  // Configure some options
  sass_option_set_precision(ctx_opt, 10);
  
  // context is all setup, call compile step now
  int status = sass_compile_file_context(file_ctx);
  
  // Print the resulCt or error to stdout
  if(status==0) puts( sass_context_get_output_string(ctx) );
  else puts( sass_context_get_error_message(ctx) );
  
  // release allocated memory
  sass_delete_file_context(file_ctx);
  
  // exit status
  return status;
}

#define SASS_INTERFACE

#include "sass.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SASS_STYLE_NESTED     0
#define SASS_STYLE_EXPANDED   1
#define SASS_STYLE_COMPACT    2
#define SASS_STYLE_COMPRESSED 3

#define SASS_SOURCE_COMMENTS_NONE 0
#define SASS_SOURCE_COMMENTS_DEFAULT 1
#define SASS_SOURCE_COMMENTS_MAP 2

#define SASS_CONTEXT_FILE   0
#define SASS_CONTEXT_FOLDER 1
#define SASS_CONTEXT_STRING 2


struct sass_options {
  int output_style;
  int source_comments; // really want a bool, but C doesn't have them
  char* include_paths;
  char* image_path;
};

struct sass_context {
  char* input;
  char* output;
  struct sass_options options;
  int error_status;
  char* error_message;
  struct Sass_C_Function_Data* c_functions;
  char** included_files;
  int num_included_files;
  char context_type;
};

struct sass_context*        sass_new_context        (void);

void sass_free_context        (struct sass_context* ctx);

int sass_compile            (struct sass_context* ctx);
int sass_compile_string     (struct sass_context* ctx);
int sass_compile_file       (struct sass_context* ctx);
int sass_compile_folder     (struct sass_context* ctx);

#ifdef __cplusplus
}
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>
#include "../sass_interface.h"

#define TEST_DATA_DIR "test/require_once_files"

char *read_file(const char *path) {
    FILE * f = fopen(path, "r");
    if (f == NULL) {
        printf("Could not open %s\n", path);
        abort();
    }
    struct stat s;
    fstat(fileno(f), &s);
    size_t fsize = s.st_size;
    char * d = malloc(s.st_size + 1);
    fread(d, fsize, sizeof (char), f);
    d[s.st_size] = 0;
    return d;
}

void init_options(struct sass_options *options, int import_once) {
    options->output_style = SASS_STYLE_COMPRESSED;
    options->source_comments = SASS_SOURCE_COMMENTS_NONE;
    options->include_paths = TEST_DATA_DIR;
    options->image_path = TEST_DATA_DIR;
    options->precision = 0;
    options->import_once = import_once;
}

void test_sass_compile_once_disabled() {
    struct sass_context *context = sass_new_context();
    init_options(&(context->options), 0);

    char *d = read_file(TEST_DATA_DIR "/a.scss");
    context->input_path = TEST_DATA_DIR "/a.scss";
    context->source_string = d;
    sass_compile(context);

    assert(0 == context->error_status);
    assert(0 == strcmp(".a{color:red;}.b{color:red;}.c{color:red;}.c{color:red;}", context->output_string));
    free(d);
    sass_free_context(context);
}

void test_sass_compile_once_enabled() {
    struct sass_context *context = sass_new_context();
    init_options(&(context->options), 1);

    char *d = read_file(TEST_DATA_DIR "/a.scss");
    context->input_path = TEST_DATA_DIR "/a.scss";
    context->source_string = d;
    sass_compile(context);
    assert(0 == context->error_status);
    assert(0 == strcmp(".a{color:red;}.b{color:red;}.c{color:red;}", context->output_string));
    free(d);
    sass_free_context(context);
}

void test_sass_compile_file_once_disabled() {
    struct sass_file_context *context = sass_new_file_context();
    init_options(&(context->options), 0);

    context->input_path = TEST_DATA_DIR "/a.scss";
    context->output_path = NULL;
    sass_compile_file(context);
    assert(0 == context->error_status);
    assert(0 == strcmp(".a{color:red;}.b{color:red;}.c{color:red;}.c{color:red;}", context->output_string));
    sass_free_file_context(context);
}

void test_sass_compile_file_once_enabled() {
    struct sass_file_context *context = sass_new_file_context();
    init_options(&(context->options), 1);

    context->input_path = TEST_DATA_DIR "/a.scss";
    context->output_path = NULL;

    sass_compile_file(context);

    assert(0 == context->error_status);
    assert(0 == strcmp(".a{color:red;}.b{color:red;}.c{color:red;}", context->output_string));
    sass_free_file_context(context);
}

int main(int argc, char *argv[]) {
    test_sass_compile_once_disabled();
    test_sass_compile_once_enabled();
    test_sass_compile_file_once_disabled();
    test_sass_compile_file_once_enabled();
    printf("PASS\n");
    return 0;
}

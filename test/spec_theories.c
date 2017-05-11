#include <stdio.h>
#include <stdlib.h>
#include <Criterion/theories.h>
#include <sass.h>
#include "tinydir/tinydir.h"
#include "slre/slre.h"

static bool skip_todo = false, unexpected_pass = false;

static void add(char ***arr, const char *value, int *length) {
  ++(*length);
  char *temp = (char *)realloc(*arr, (*length) * sizeof(**arr));

  if (!temp) {
    --(*length);
    return;
  }

  *arr = temp;
  (*arr)[*length - 1] = (char *)malloc(strlen(value) * sizeof(char) + 1);
  strcpy((*arr)[*length - 1], value);
}

static void empty(char ***arr, int length) {
  for (int i = 0; i < length; ++i) {
    free((*arr)[i]);
  }
}

// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
  char *result; // the return string
  char *ins;    // the next insert point
  char *tmp;    // varies
  int len_rep;  // length of rep
  int len_with; // length of with
  int len_front; // distance between rep and end of last rep
  int count;    // number of replacements

  if (!orig)
    return NULL;
  if (!rep)
    rep = "";
  len_rep = strlen(rep);
  if (!with)
    with = "";
  len_with = strlen(with);

  ins = orig;
  for (count = 0; tmp = strstr(ins, rep); ++count) {
    ins = tmp + len_rep;
  }

  // first time through the loop, all the variable are set correctly
  // from here on,
  //    tmp points to the end of the result string
  //    ins points to the next occurrence of rep in orig
  //    orig points to the remainder of orig after "end of rep"
  tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

  if (!result)
    return NULL;

  while (count--) {
    ins = strstr(orig, rep);
    len_front = ins - orig;
    tmp = strncpy(tmp, orig, len_front) + len_front;
    tmp = strcpy(tmp, with) + len_with;
    orig += len_front + len_rep; // move to next "end of rep"
  }
  strcpy(tmp, orig);
  return result;
}

static void recursively_collect(char ***arr, const char *path, int *len) {
  tinydir_dir dir;

  tinydir_open(&dir, path);

  while (dir.has_next) {
    tinydir_file file;
    if (tinydir_readfile(&dir, &file) == -1) {
      perror("Error getting file");
      break;
    }
    if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0) {
      tinydir_next(&dir);
      continue;
    }

    if (file.is_dir && (!skip_todo || !strstr(file.name, "-todo-"))) {
      recursively_collect(arr, file.path, len);
    }
    else if (strcmp(file.name, "input.scss") == 0 || strcmp(file.name, "input.sass") == 0) {
      add(arr, file.path, len);
    }

    tinydir_next(&dir);
  }

  tinydir_close(&dir);
}

static char *read_file(const char *path) {
  FILE *fp;
  long lSize;
  char *buffer;

  fp = fopen(path, "rb");
  if (!fp) perror(path), exit(1);

  fseek(fp, 0L, SEEK_END);
  lSize = ftell(fp);
  rewind(fp);

  buffer = calloc(1, lSize + 1);
  if (!buffer) fclose(fp), fputs("memory alloc fails", stderr), exit(1);

  fread(buffer, lSize, 1, fp);
  fclose(fp);

  return buffer;
}

static void list_directories(void ***out, size_t *size) {
  int len = 0;
  char *spec_dir = str_replace(__FILE__, "spec_theories.c", "sass-spec/spec");
  char *todo_var = getenv("LIBSASS_SKIP_TODO");
  char *unexpected_pass_var = getenv("LIBSASS_UNEXPECTED_PASS");

  skip_todo = !strcmp("1", (todo_var ? todo_var : "0"));
  unexpected_pass = !strcmp("1", (unexpected_pass_var ? unexpected_pass_var : "0"));

  (*out) = malloc(sizeof(char*));
  recursively_collect(&(*out), spec_dir, &len);
  free(spec_dir);

  (*size) = len;
}

TheoryDataPoints(theory, gen) = {
  DataPoints(const char *, "") // parameter placeholder
};

static void generate_datapoints(void) {
  list_directories(&TheoryDataPoint(theory, gen)[0].arr, &TheoryDataPoint(theory, gen)[0].len);
}

static void free_datapoints(void) {
  empty(&TheoryDataPoint(theory, gen)[0].arr, TheoryDataPoint(theory, gen)[0].len);
  free(TheoryDataPoint(theory, gen)[0].arr);
}

static char *slre_replace_alternative_dynamic_capture_version(const char *buf, const char *regex, const char *sub, const int num) {
  char *s = NULL;
  int n, n1, n2, n3, s_len, len = strlen(buf);
  struct slre_cap *cap = malloc(sizeof(struct slre_cap) * num);// = { NULL, 0 };

  do {
    s_len = s == NULL ? 0 : strlen(s);
    if ((n = slre_match(regex, buf, len, cap, num, 0)) > 0) {
      n1 = cap[0].ptr - buf, n2 = strlen(sub),
        n3 = &buf[n] - &cap[0].ptr[cap[0].len];
    }
    else {
      n1 = len, n2 = 0, n3 = 0;
    }
    s = (char *)realloc(s, s_len + n1 + n2 + n3 + 1);
    memcpy(s + s_len, buf, n1);
    memcpy(s + s_len + n1, sub, n2);
    memcpy(s + s_len + n1 + n2, cap[0].ptr + cap[0].len, n3);
    s[s_len + n1 + n2 + n3] = '\0';

    buf += n > 0 ? n : len;
    len -= n > 0 ? n : len;
  } while (len > 0);

  free(cap);
  return s;
}

static char *slre_replace(const char *buf, const char *regex, const char *sub) {
  char *s = NULL;
  int n, n1, n2, n3, s_len, len = strlen(buf);
  struct slre_cap cap = { NULL, 0 };

  do {
    s_len = s == NULL ? 0 : strlen(s);
    if ((n = slre_match(regex, buf, len, &cap, 1, 0)) > 0) {
      n1 = cap.ptr - buf, n2 = strlen(sub),
        n3 = &buf[n] - &cap.ptr[cap.len];
    }
    else {
      n1 = len, n2 = 0, n3 = 0;
    }
    s = (char *)realloc(s, s_len + n1 + n2 + n3 + 1);
    memcpy(s + s_len, buf, n1);
    memcpy(s + s_len + n1, sub, n2);
    memcpy(s + s_len + n1 + n2, cap.ptr + cap.len, n3);
    s[s_len + n1 + n2 + n3] = '\0';

    buf += n > 0 ? n : len;
    len -= n > 0 ? n : len;
  } while (len > 0);

  return s;
}

static char *sanitize_str(const char *input) {
  char *s1 = slre_replace(input, "(\t+)", " ");
  char *s2 = slre_replace(s1, "(\r+)", " ");
  char *s3 = slre_replace(s2, "(\n+)", " ");
  char *s4 = slre_replace(s3, "(\f+)", " ");
  char *s5 = slre_replace(s4, "( +)", " ");
  char *s6 = slre_replace(s5, "( *\{)", " {\n");
  char *s7 = slre_replace(s6, "(; *)", ";\n");
  char *s8 = slre_replace(s7, "(, *)", ",\n");
  char *s9 = slre_replace(s8, "( *\} *)", " }\n");
  char *s10 = slre_replace(s9, "((\r?\n)*;(\r?\n)*(?(\r?\n)*:(\r?\n)*\s*(\r?\n)*;(\r?\n)*)+(\r?\n)*)", ";");
  char *s11 = slre_replace(s10, "((\r?\n)*;(\r?\n)* *(\r?\n)*})", "; }");

  free(s1);
  free(s2);
  free(s3);
  free(s4);
  free(s5);
  free(s6);
  free(s7);
  free(s8);
  free(s9);
  free(s10);

  return s11;
}

Theory((const char *input_path), theory, gen, .init = generate_datapoints, .fini = free_datapoints) {
  char *output_path = strstr(input_path, "input.scss") ?
    str_replace(input_path, "input.scss", "expected_output.css") :
    str_replace(input_path, "input.sass", "expected_output.css");

  /* TODO: Needs an efficient way to repeat the process for these:
  char *output_compact = str_replace(str_replace(input_path, "input.scss", "expected.compact.css"), "input.sass", "expected.compact.css");
  char *output_compressed = str_replace(str_replace(input_path, "input.scss", "expected.compressed.css"), "input.sass", "expected.compressed.css");
  char *output_expanded = str_replace(str_replace(input_path, "input.scss", "expected.expanded.css"), "input.sass", "expected.expanded.css");
  */

  struct Sass_File_Context* ctx = sass_make_file_context(input_path);
  struct Sass_Context* ctx_out = sass_file_context_get_context(ctx);
  struct Sass_Options* options = sass_make_options();

  sass_option_set_output_style(options, SASS_STYLE_NESTED);
  sass_option_set_output_path(options, output_path);
  sass_option_set_input_path(options, input_path);
  sass_file_context_set_options(ctx, options);
  sass_compile_file_context(ctx);

  int error_status = sass_context_get_error_status(ctx_out);
  const char *error_message = sass_context_get_error_message(ctx_out);
  const char *actual = sass_context_get_output_string(ctx_out);
  char *expected = read_file(output_path);
  bool is_todo = skip_todo ? false : strstr(input_path, "-todo-");
  bool is_match;

  if (error_status) {
    is_match = !strcmp(expected, error_message);
  }
  else {
    char *sanitized_expected = sanitize_str(expected);
    char *sanitized_actual = sanitize_str(actual);

    is_match = !strcmp(sanitized_expected, sanitized_actual);

    free(sanitized_expected);
    free(sanitized_actual);
  }

  if (unexpected_pass && is_todo) {
    cr_assert(!is_match, "\n[%s] -> passed a test we expected it to fail\n\n", output_path);
  }
  else {
    cr_assert(is_match, "\n[%s] -> Expected did not match output\n\n", output_path);
  }

  free(output_path);
  free(expected);
  sass_delete_file_context(ctx);
}

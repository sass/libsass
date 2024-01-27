/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "capi_getopt.hpp"

#include <cstring>
#include <iomanip>
#include <algorithm>
#include "exceptions.hpp"
#include "string_utils.hpp"
#include "capi_compiler.hpp"

using namespace Sass;

/////////////////////////////////////////////////////////////////////////
// This file implements something very similar to GNU (long) getopt.
// It supports parsing a list of string arguments to configure a compiler.
// Corresponds exactly to how SassC will parse `argv` command line
// arguments and makes this feature available to all implementors.
/////////////////////////////////////////////////////////////////////////
// So far we support boolean, string and enumeration options and we
// might extend this list if the need arises. Boolean options don't
// allow any argument, but can be inverted with the `--no-` prefix.
// Other options may have an additional argument and also a default.
/////////////////////////////////////////////////////////////////////////
// Some of the key features are:
// - Support for boolean options with [--no-] prefix
// - Short options don't support exclamation `!` mark yet
// - Supports name shortening, if target can be identified uniquely
/////////////////////////////////////////////////////////////////////////
// You may also use this API to completely use different or additional
// options. Although part of LibSass it could also be used standalone.
// APIs are quite raw and not optimized for multi purpose though.
/////////////////////////////////////////////////////////////////////////


// Single enumeration item for sass options
// Maps an option to the given enum integer.
struct SassGetOptEnum
{
public:
  int enumid;
  const char* string;
  SassGetOptEnum(const char* name, int id) :
    enumid(id),
    string(name)
  {}
};

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

class SassOption {
public:
  const char shrt = '\0';
  const char* name;
  const char* desc;
  const bool boolean = false;
  const char* argument = nullptr;
  const bool optional = false;
  const struct SassGetOptEnum* enums;
  void (*cb) (struct SassGetOpt* getopt, union SassOptionValue value);
  SassOption(
    const char shrt = '\0',
    const char* name = nullptr,
    const char* desc = nullptr,
    const bool boolean = false,
    const char* argument = nullptr,
    const bool optional = false,
    const struct SassGetOptEnum* enums = nullptr,
    void (*cb) (struct SassGetOpt* getopt, union SassOptionValue value) = nullptr
  ) :
    shrt(shrt),
    name(name),
    desc(desc),
    boolean(boolean),
    argument(argument),
    optional(optional),
    enums(enums),
    cb(cb)
  {}
};

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

struct SassArgument {
  bool optional = false;
  const char* name;
  void (*cb) (struct SassGetOpt* getopt, const char* arg);
  SassArgument(
    bool optional = false,
    const char* name = nullptr,
    void (*cb) (struct SassGetOpt* getopt, const char* arg) = nullptr
  ) :
    optional(optional),
    name(name),
    cb(cb)
  {}
};

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

struct SassGetOpt {
  Compiler& compiler;
  sass::string wasAssignment;
  bool lastArgWasShort = false;
  bool needsArgumentWasShort = false;
  const SassOption* lastArg = nullptr;
  const SassOption* needsArgument = nullptr;
  sass::vector<sass::string> args = {};
  sass::vector<SassOption> options;
  sass::vector<struct SassArgument> arguments;
  SassGetOpt(Compiler& compiler) :
    compiler(compiler)
  {}
};

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Enums for input format option
const struct SassGetOptEnum format_options[] = {
  { "scss", SASS_IMPORT_SCSS },
  { "sass", SASS_IMPORT_SASS },
  { "css", SASS_IMPORT_CSS },
  { "auto", SASS_IMPORT_AUTO },
  { nullptr, 0 }
};

// Enums for output style option
const struct SassGetOptEnum style_options[] = {
  { "nested", SASS_STYLE_NESTED },
  { "expanded", SASS_STYLE_EXPANDED },
  { "compact", SASS_STYLE_COMPACT },
  { "compressed", SASS_STYLE_COMPRESSED },
  { nullptr, 0 }
};

// Enums for source-map mode option
const struct SassGetOptEnum srcmap_options[] = {
  { "none", SASS_SRCMAP_NONE },
  { "create", SASS_SRCMAP_CREATE },
  { "link", SASS_SRCMAP_EMBED_LINK },
  { "embed", SASS_SRCMAP_EMBED_JSON },
  { nullptr, 0 }
};

// Simple proxy functions to call out to compiler (or set certain options directly)
void getopt_set_input_format(struct SassGetOpt* getopt, union SassOptionValue value) { getopt->compiler.input_syntax = value.syntax; }
void getopt_set_output_style(struct SassGetOpt* getopt, union SassOptionValue value) { getopt->compiler.output_style = value.style; }
void getopt_add_include_path(struct SassGetOpt* getopt, union SassOptionValue value) { getopt->compiler.addIncludePaths(value.string); }
void getopt_load_plugins(struct SassGetOpt* getopt, union SassOptionValue value) { getopt->compiler.loadPlugins(value.string); }
void getopt_set_srcmap_mode(struct SassGetOpt* getopt, union SassOptionValue value) { getopt->compiler.mapopt.mode = value.mode; }
void getopt_set_srcmap_file_urls(struct SassGetOpt* getopt, union SassOptionValue value) { getopt->compiler.mapopt.file_urls = value.boolean; }
void getopt_set_srcmap_contents(struct SassGetOpt* getopt, union SassOptionValue value) { getopt->compiler.mapopt.embed_contents = value.boolean; }
void getopt_set_srcmap_root(struct SassGetOpt* getopt, union SassOptionValue value) { getopt->compiler.mapopt.root = value.boolean; }
void getopt_set_srcmap_path(struct SassGetOpt* getopt, union SassOptionValue value) { getopt->compiler.mapopt.path = value.boolean; }
void getopt_set_term_unicode(struct SassGetOpt* getopt, union SassOptionValue value) { getopt->compiler.support_unicode = value.boolean; }
void getopt_set_term_colors(struct SassGetOpt* getopt, union SassOptionValue value) { getopt->compiler.support_colors = value.boolean; }
void getopt_set_suppress_stderr(struct SassGetOpt* getopt, union SassOptionValue value) { getopt->compiler.suppress_stderr = true; }

void getopt_error(struct SassGetOpt* getopt, const char* what)
{
  if (getopt) {
    handle_error(getopt->compiler, 9, what, nullptr);
  }
}

// Precision setter has specific validation (no corresponding type in GetOpt parser)
void getopt_set_precision(struct SassGetOpt* getopt, union SassOptionValue value)
{
  // The GetOpt API does not yet know integers
  try {
    // Try to convert the precision string to int
    getopt->compiler.setPrecision(std::stoi(value.string));
  }
  catch (std::exception&) {
    getopt_error(getopt, "option '--precision' is not a valid integer");
    return; // return after error
  }
}

void cli_sass_compiler_set_line_numbers(struct SassGetOpt* getopt, union SassOptionValue value) {
  std::cerr << "cli_sass_compiler_set_line_numbers " << value.boolean << "\n";
}


void getopt_print_help(struct SassGetOpt* getopt, std::ostream& stream);

void cli_sass_compiler_version(struct SassGetOpt* getopt, union SassOptionValue value) {
  getopt_print_help(getopt, std::cerr);
  exit(0);
}

void cli_sass_compiler_help(struct SassGetOpt* getopt, union SassOptionValue value) {
  getopt_print_help(getopt, std::cerr);
  exit(0);
}

void cli_sass_compiler_input_file_arg(struct SassGetOpt* getopt, const char* path)
{
  struct SassImport* entry = strncmp(path, "--", 2) == 0
    ? sass_make_stdin_import("stream://stdin")
    : sass_make_file_import(path);
  sass_compiler_set_entry_point(getopt->compiler.wrap(), entry);
  sass_delete_import(entry);
}

void cli_sass_compiler_output_file_arg(struct SassGetOpt* getopt, const char* path)
{
  sass_compiler_set_output_path(getopt->compiler.wrap(), path);
}


sass::string format_option(struct SassGetOpt* getopt, SassOption& option)
{
  Compiler& compiler(getopt->compiler);
  sass::sstream line;
  if (option.shrt) {
    line << compiler.getTerm(Terminal::bold_magenta);
    line << "-" << option.shrt;
    line << compiler.getTerm(Terminal::reset);
    if (option.name) line << ", ";
  }
  else {
    line << "    ";
  }
  if (option.name) {
    line << compiler.getTerm(Terminal::green);
    line << "--";
    line << compiler.getTerm(Terminal::reset);
    if (option.boolean) {
      line << compiler.getTerm(Terminal::blue);
      line << "[no-]";
      line << compiler.getTerm(Terminal::reset);
    }
    line << compiler.getTerm(Terminal::green);
    line << option.name;
    line << compiler.getTerm(Terminal::reset);
  }
  if (option.argument) {
    if (option.optional) line << "[";
    line << "=";
    line << compiler.getTerm(Terminal::cyan);
    line << option.argument;
    line << compiler.getTerm(Terminal::reset);
    if (option.optional) line << "]";
  }
  return line.str();
}

void getopt_print_help(struct SassGetOpt* getopt, std::ostream& stream)
{
  size_t longest = 20;
  // Determine longest option to align them all
  for (SassOption& option : getopt->options) {
    sass::string fmt(format_option(getopt, option));
    size_t len = Terminal::count_printable(fmt.c_str()) + 2;
    if (len > longest) longest = len;
  }
  // Print out each option line by line
  for (SassOption& option : getopt->options) {
    sass::string fmt(format_option(getopt, option));
    size_t len = Terminal::count_printable(fmt.c_str());
    stream << "  " << fmt;
    stream.width(longest - len + 2);
    stream << " " << option.desc;
    stream << "\n";
    if (option.enums) {
      auto enums = option.enums;
      sass::vector<sass::string> names;
      while (enums && enums->string) {
        names.push_back(enums->string);
        enums += 1;
      }
      stream << std::setw(longest + 4) << "";
      if (option.argument) {
        stream << getopt->compiler.getTerm(Terminal::cyan);
        stream << option.argument;
        stream << getopt->compiler.getTerm(Terminal::reset);
        stream << " must be ";
      }
      stream << toSentence(names, "or",
        getopt->compiler.getTerm(Terminal::yellow),
        getopt->compiler.getTerm(Terminal::reset),
      '\'') << "\n";
    }
  }
}

sass::vector<const SassOption*> find_short_options(struct SassGetOpt* getopt, const char arg)
{
  sass::vector<const SassOption*> matches;
  for (SassOption& option : getopt->options) {
    if (option.shrt == arg) {
      matches.push_back(&option);
    }
  }
  return matches;
}

sass::vector<const SassOption*> find_long_options(struct SassGetOpt* getopt, const sass::string& arg)
{
  sass::vector<const SassOption*> matches;
  for (const SassOption& option : getopt->options) {
    if (StringUtils::startsWithIgnoreCase(option.name, arg)) {
      if (arg == option.name) return { &option };
      matches.push_back(&option);
    }
    if (option.boolean) {
      if (StringUtils::startsWithIgnoreCase(arg, "no-", 3)) {
        sass::string name(arg.begin() + 3, arg.end());
        if (StringUtils::startsWithIgnoreCase(option.name, name)) {
          if (arg == option.name) return { &option };
          matches.push_back(&option);
        }
      }
    }
  }
  return matches;
}

sass::vector<const struct SassGetOptEnum*> find_options_enum(
  const struct SassGetOptEnum* enums, const sass::string& arg)
{
  sass::vector<const struct SassGetOptEnum*> matches;
  while (enums && enums->string) {
    if (StringUtils::startsWithIgnoreCase(enums->string, arg)) {
      matches.push_back(enums);
    }
    enums += 1;
  }
  return matches;
}

// Check for too many or not enough arguments
// Skip this check if nothing is expected at all
// This will simply store the arguments in `args`
void getopt_check_and_consume_arguments(struct SassGetOpt* getopt)
{
  if (getopt->compiler.state) return;
  if (getopt->arguments.empty()) return;
  size_t want_size = getopt->arguments.size();
  size_t requires = getopt->arguments.size();
  for (const auto& arg : getopt->arguments) {
    if (arg.optional) requires -= 1;
  }
  size_t have_size = getopt->args.size();
  for (size_t i = 0; i < have_size; i += 1) {
    if (want_size <= i) {
      sass::sstream strm;
      sass::string value(getopt->args[i]);
      StringUtils::makeReplace(value, "'", "\\'");
      strm << "extra argument '" << value << "'";
      sass::string msg(strm.str());
      getopt_error(getopt, msg.c_str());
      return; // return after error
    }
    else {
      // Call back to registered handler
      getopt->arguments[i].cb(getopt, getopt->args[i].c_str());
    }
  }
  for (size_t i = have_size; i < requires; i += 1) {
    sass::sstream strm;
    sass::string value(getopt->arguments[i].name);
    StringUtils::makeReplace(value, "'", "\\'");
    strm << "missing required argument '" << value << "'";
    sass::string msg(strm.str());
    getopt_error(getopt, msg.c_str());
    return; // return after error
  }

}

// Check for pending required option
void getopt_check_required_option(struct SassGetOpt* getopt)
{
  if (getopt->compiler.state) return;
  // Expected argument, error
  if (getopt->needsArgument) {
    sass::sstream strm;
    if (getopt->needsArgumentWasShort) {
      sass::string value(1, getopt->needsArgument->shrt);
      // StringUtils::makeReplace(value, "'", "\\'"); // only a char
      strm << "option '-" << value << "' requires an argument'";
    }
    else {
      sass::string value(getopt->needsArgument->name);
      StringUtils::makeReplace(value, "'", "\\'");
      strm << "option '--" << value << "' requires an argument'";
    }
    sass::string msg(strm.str());
    getopt_error(getopt, msg.c_str());
    return; // return after error
  }
}

// Function that must be consecutively called for every argument.
// Ensures to properly handle cases where a mandatory or optional
// argument, if required by the previous option, is handled correctly.
// This is a bit different to "official" GNU GetOpt, but should be
// reasonably well and support more advanced usages than before.
void getopt_parse(struct SassGetOpt* getopt, const char* value)
{
  if (value == nullptr) return;
  if (getopt->compiler.state) return;
  sass::string arg(value);
  StringUtils::makeTrimmed(arg);
  union SassOptionValue result {};

  if (arg != "-" && arg != "--" &&
    arg[0] == '-' && getopt->wasAssignment.empty())
  {
    getopt_check_required_option(getopt);
    sass::vector<const SassOption*> opts;

    // Check if we have some assignment
    size_t assign = arg.find_first_of('=');
    if (assign != std::string::npos) {
      sass::string key(arg.begin(), arg.begin() + assign);
      sass::string val(arg.begin() + assign + 1, arg.end());
      getopt_parse(getopt, key.c_str());
      getopt->wasAssignment = key;
      getopt_parse(getopt, val.c_str());
      getopt->wasAssignment.clear();
      return;
    }

    // Long argument
    if (arg[1] == '-') {
      arg.erase(0, 2);
      opts = find_long_options(getopt, arg);
      getopt_check_required_option(getopt);
    }
    // Short argument
    else {
      arg.erase(0, 1);
      // Split multiple short args
      if (arg.size() > 1) {
        for (size_t i = 0; i < arg.size(); i += 1) {
          sass::string split("-"); split += arg[i];
          sass_getopt_parse(getopt, split.c_str());
          getopt_check_required_option(getopt);
          // break on first error
        }
        return;
      }
      opts = find_short_options(getopt, arg[0]);
      // Consume further until has arg
    }
    if (opts.size() == 0) {
      sass::sstream strm;
      strm << "unrecognized option '--" << arg << "'";
      sass::string msg(strm.str());
      getopt_error(getopt, msg.c_str());
      return; // return after error
    }
    if (opts.size() > 1) {
      sass::sstream strm;
      if (value[0] == '-' && value[1] == '-') {
        strm << "option '--" << arg << "' is ambiguous; possibilities: ";
        for (auto opt : opts) strm << "'--" << opt->name << "'" << std::setw(4);
      }
      else {
        // Should never happen if you configured your options right
        // Triggered by sassc.exe -MP
        strm << "option '-" << arg << "' is ambiguous1 (internal error)";
        for (auto opt : opts) strm << "'--" << opt->name << "'" << std::setw(4);
      }
      sass::string msg(strm.str());
      getopt_error(getopt, msg.c_str());
      return; // return after error
    }
    getopt->lastArg = opts[0];
    getopt->needsArgument = opts[0]->argument ? opts[0] : nullptr;
    getopt->needsArgumentWasShort = value[0] == '-' && value[1] != '-';

    // Check boolean options right away
    if (opts[0]->boolean) {
      // Get boolean result (invert if argument has "no-" prefix)
      result.boolean = !StringUtils::startsWithIgnoreCase(arg, "no-", 3);
    }
    if (!getopt->needsArgument) {
      opts[0]->cb(getopt, result);
    }
  }
  else if (getopt->needsArgument) {
    if (getopt->needsArgument->enums) {
      auto matches = find_options_enum(
        getopt->needsArgument->enums, arg);
      if (matches.size() == 0) {
        sass::sstream strm;
        strm << "enum '" << arg << "' is not valid for option '";
        if (getopt->needsArgumentWasShort) {
          strm << "-" << getopt->needsArgument->shrt;
        }
        else {
          strm << "--" << getopt->needsArgument->name;
        }
        strm << "' (valid enums are ";
        auto enums = getopt->needsArgument->enums;
        sass::vector<sass::string> names;
        while (enums && enums->string) {
          names.push_back(enums->string);
          enums += 1;
        }
        strm << Sass::toSentence(names, "or",
          getopt->compiler.getTerm(Terminal::yellow),
          getopt->compiler.getTerm(Terminal::reset), '\'') << ")";
        sass::string msg(strm.str());
        getopt_error(getopt, msg.c_str());
        return; // return after error
      }
      else if (matches.size() > 1) {

        sass::sstream strm;
        strm << "enum '" << arg << "' for option '";
        if (getopt->needsArgumentWasShort) {
          strm << "-" << getopt->needsArgument->shrt;
        }
        else {
          strm << "--" << getopt->needsArgument->name;
        }
        strm << "' is ambiguous (possibilities are ";
        sass::vector<sass::string> names;
        for (auto match : matches) {
          names.push_back(match->string);
        }
        strm << toSentence(names, "or",
          getopt->compiler.getTerm(Terminal::yellow),
          getopt->compiler.getTerm(Terminal::reset), '\'') << ")";
        sass::string msg(strm.str());
        getopt_error(getopt, msg.c_str());
        return; // return after error
      }
      result.integer = matches[0]->enumid;
    }
    else {
      result.string = arg.c_str();
    }
    getopt->needsArgument->cb(getopt, result);
    getopt->needsArgumentWasShort = false;
    getopt->needsArgument = nullptr;
  }
  else if (!getopt->wasAssignment.empty()) {
    sass::sstream strm;
    strm << "option '";
    if (getopt->lastArgWasShort) {
      strm << "-" << getopt->lastArg->shrt;
    }
    else {
      strm << "--" << getopt->lastArg->name;
    }
    strm << "' doesn't allow an argument";
    sass::string msg(strm.str());
    getopt_error(getopt, msg.c_str());
    return; // return after error
  }
  else {
    // This is a regular argument
    getopt->args.push_back(arg);
  }

}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

extern "C" {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Create a new GetOpt parser to help with parsing config from users
  // Optimized to act like GNU GetOpt Long to consume `argv` items
  // But can also be used to parse any other list of config strings
  struct SassGetOpt* ADDCALL sass_make_getopt(struct SassCompiler* compiler)
  {
    return new struct SassGetOpt(Compiler::unwrap(compiler));
  }
  // EO sass_make_getopt

  void ADDCALL sass_getopt_parse(struct SassGetOpt* getopt, const char* value)
  {
    getopt_parse(getopt, value);
  }

  // Return string with the full help message describing all commands
  // This is formatted in a similar fashion as GNU tools using getopt
  char* ADDCALL sass_getopt_get_help(struct SassGetOpt* getopt)
  {
    sass::sstream usage;
    getopt_print_help(getopt, usage);
    return sass_copy_string(usage.str());
  }
  // EO sass_getopt_get_help

  // Delete and finalize the GetOpt parser. Make sure to call
  // this before you want to start the actual compilation phase.
  void ADDCALL sass_delete_getopt(struct SassGetOpt* getopt)
  {
    // Check for pending required option
    getopt_check_required_option(getopt);
    // Check for too many or not enough arguments
    // Skip this check if nothing is expected at all
    // This will simply store the arguments in `args`
    getopt_check_and_consume_arguments(getopt);
    // Release memory
    delete getopt;
  }
  // EO sass_delete_getopt

  // Register additional option that can be parsed
  void ADDCALL sass_getopt_register_option(struct SassGetOpt* getopt,
    // Short and long parameter names
    const char short_name,
    const char* long_name,
    // Description used in help/usage message
    const char* description,
    // Whether to act like a boolean
    const bool boolean,
    // Name of required argument
    const char* argument,
    // Make argument optional
    const bool optional,
    // Arguments must be one of this enum
    const struct SassGetOptEnum* enums,
    // Callback function, where we pass back the given option value
    void (*cb) (struct SassGetOpt* getopt, union SassOptionValue value))
  {
    getopt->options.emplace_back(SassOption{ short_name, long_name,
      description, boolean, argument, optional, enums, cb });
  }
  // EO sass_getopt_register_option

  // Register additional argument that can be parsed
  void ADDCALL sass_getopt_register_argument(struct SassGetOpt* getopt,
    // Whether this argument is optional
    bool optional,
    // Name used in messages
    const char* name,
    // Callback function, where we pass back the given argument value
    void (*cb) (struct SassGetOpt* getopt, const char* value))
  {
    getopt->arguments.emplace_back(SassArgument{ optional, name, cb });
  }
  // EO sass_getopt_register_argument

  // Utility function to tell LibSass to register its default options
  void ADDCALL sass_getopt_populate_options(struct SassGetOpt* getopt)
  {

    /* enum: style */ sass_getopt_register_option(getopt, 't', "style", "Set output style (nested, expanded, compact or compressed).", false, "STYLE", false, style_options, getopt_set_output_style);
    /* enum: format */ sass_getopt_register_option(getopt, 'f', "format", "Set explicit input syntax (scss, sass, css or auto).", false, "SYNTAX", true, format_options, getopt_set_input_format);
    /* path */ sass_getopt_register_option(getopt, 'I', "include-path", "Add include path to look for imports.", false, "PATH", false, nullptr, getopt_add_include_path);
    /* path */ sass_getopt_register_option(getopt, 'P', "plugin-path", "Add plugin path to auto load plugins.", false, "PATH", false, nullptr, getopt_load_plugins);
    /* enum: mode */ sass_getopt_register_option(getopt, 'm', "sourcemap", "Set how to create and emit source mappings.", false, "TYPE", true, srcmap_options, getopt_set_srcmap_mode);
    /* bool */ sass_getopt_register_option(getopt, '\0', "sourcemap-file-urls", "Emit absolute file:// urls in includes array.", true, nullptr, true, nullptr, getopt_set_srcmap_file_urls);
    /* bool */ sass_getopt_register_option(getopt, 'C', "sourcemap-contents", "Embed contents of imported files in source map.", true, nullptr, true, nullptr, getopt_set_srcmap_contents);
    /* path */ sass_getopt_register_option(getopt, 'M', "sourcemap-path", "Set path where source map file is saved.", false, "PATH", false, nullptr, getopt_set_srcmap_path);
    /* int */ sass_getopt_register_option(getopt, 'p', "precision", "Set floating-point precision for numbers.", false, "{0-12}", false, nullptr, getopt_set_precision);
    /* bool */ // sass_getopt_register_option(getopt, '\0', "unicode", "Enable or disable unicode in generated css output.", true, nullptr, false, nullptr, getopt_set_unicode);
    /* bool */ sass_getopt_register_option(getopt, 'l', "line-comments", "Emit comments showing original line numbers.", true, nullptr, false, nullptr, cli_sass_compiler_set_line_numbers);
    /* bool */ sass_getopt_register_option(getopt, '\0', "term-unicode", "Enable or disable terminal unicode output.", true, nullptr, false, nullptr, getopt_set_term_unicode);
    /* bool */ sass_getopt_register_option(getopt, '\0', "term-colors", "Enable or disable terminal ANSI color output.", true, nullptr, false, nullptr, getopt_set_term_colors);
    /* bool */ sass_getopt_register_option(getopt, '\0', "quiet", "Do not print any warnings to stderr.", false, nullptr, false, nullptr, getopt_set_suppress_stderr);

  }
  // EO sass_getopt_populate_options

  // Utility function to tell LibSass to register its default arguments
  void ADDCALL sass_getopt_populate_arguments(struct SassGetOpt* getopt)
  {
    sass_getopt_register_argument(getopt, false, "INPUT_FILE|--", cli_sass_compiler_input_file_arg);
    sass_getopt_register_argument(getopt, true, "OUTPUT_FILE|--", cli_sass_compiler_output_file_arg);
  }
  // EO sass_getopt_populate_arguments

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

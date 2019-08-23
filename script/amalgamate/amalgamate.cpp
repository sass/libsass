#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_map>
#include <unordered_set>

#include "filesystem-polyfill/ghc/filesystem.hpp"

// #define DEBUG 1

namespace {

namespace fs = ghc::filesystem;

std::string ReadFile(const fs::path &path) {
  std::ifstream ifs(path, std::ios::binary);
  return std::string(std::istreambuf_iterator<char>(ifs), {});
}

struct IncludeStatement {
  std::string relpath;
  std::size_t line_begin;
  std::size_t line_end;
};

struct FileData {
  std::string contents;
  bool header;
  std::vector<IncludeStatement> includes;
};

class Amalgamator {
 public:
  Amalgamator(std::vector<fs::path> src_dirs, std::vector<std::string> exts,
              std::unordered_set<std::string> exclude, bool inline_sources)
      : src_dirs_(std::move(src_dirs)),
        exts_(exts),
        exclude_(exclude),
        inline_sources_(inline_sources) {}

  void Amalgamate(std::ostream &out) {
    LoadFiles();

#ifdef DEBUG
    const auto log_strings = [](const char *name,
                                const std::vector<std::string> &xs) {
      std::cerr << name << " (" << xs.size() << "):";
      for (const std::string &x : xs) std::cerr << " " << x;
      std::cerr << std::endl;
    };

    log_strings("Files", files_);
#endif

    if (files_.empty()) {
      throw std::runtime_error("Could not find any files to amalgamate");
    }

    if (inline_sources_) {
      for (auto &it : files_data_) AnalyzeIncludes(it.first, &it.second);
      std::unordered_set<std::string> written;
      for (const auto &file : files_) {
        WriteReplaceIncludes(out, file, /*parent=*/"", &written);
      }
    } else {
      for (const auto &file : files_) {
        if (files_data_[file].header) continue;
        out << "#include \"" << file << "\"" << std::endl;
      }
    }
    out.flush();
  }

 private:
  void WriteReplaceIncludes(std::ostream &out, const std::string relpath,
                            const std::string &parent,
                            std::unordered_set<std::string> *written) {
    if (written->find(relpath) != written->end()) return;
    written->insert(relpath);
    out << "/* AMALGAM: " << relpath;
    if (!parent.empty()) out << " included from " << parent;
    out << " */ \n";
    const auto &data = files_data_.at(relpath);
    std::size_t prev = 0;
    for (const auto &incl : data.includes) {
      out.write(data.contents.data() + prev, incl.line_begin - prev);
      WriteReplaceIncludes(out, incl.relpath, relpath, written);
      prev = incl.line_end;
    }
    out.write(data.contents.data() + prev, data.contents.size() - prev);
    if (data.contents.empty() || data.contents.back() != '\n') {
      out.write("\n", 1);
    }
  }

  void LoadFiles() {
    for (const auto &src_dir : src_dirs_) {
      for (const auto &entry : fs::recursive_directory_iterator(src_dir)) {
        if (!entry.is_regular_file()) continue;
        const auto ext = entry.path().extension().u8string();
        if (std::find(exts_.begin(), exts_.end(), ext) == exts_.end()) continue;

        std::string relpath =
            fs::relative(entry.path(), src_dir).generic_u8string();
        if (exclude_.find(relpath) != exclude_.end()) continue;

        files_.emplace_back(relpath);
        files_data_[std::move(relpath)] = {ReadFile(entry.path()),
                                           ext[1] == 'h'};
      }
    }
    std::sort(files_.begin(), files_.end());
  }

  void AnalyzeIncludes(const std::string &relpath, FileData *data) {
    static const auto *const kIncludeRegex =
        new std::regex(R"([ \t]*#include (<[^"\n>]*>|"[^"\n]*")[^\n]*\n)",
                       std::regex::optimize);
    const char *const s = data->contents.c_str();
    std::size_t pos = 0;
    std::unordered_set<std::string> found_includes;
    for (std::cmatch m; std::regex_search(s + pos, m, *kIncludeRegex);
         pos += m.position() + m.length()) {
      // Match only beginning of line:
      if (!(pos + m.position() == 0 || s[pos + m.position() - 1] == '\n')) {
        continue;
      }
      if (m[1].length() < 3) {
        throw std::runtime_error("Invalid include: " + m.str());
      }
      std::string include(m[1].first + 1, m[1].length() - 2);

      auto found = ResolveInclude(relpath, include);
      if (found.empty()) continue;
      if (found == relpath) {
        std::cerr << "WARNING: Self-include in " << relpath << std::endl;
        continue;
      }
      if (found_includes.find(found) != found_includes.end()) {
        std::cerr << "WARNING: Duplicate #include of " << found << " in "
                  << relpath << std::endl;
      }
      found_includes.insert(found);
      data->includes.push_back({std::move(found), pos + m.position(),
                                pos + m.position() + m.length()});
    }

#ifdef DEBUG
    if (data->includes.empty()) return;
    std::cerr << "Includes for " << relpath << ":";
    for (const auto &x : data->includes)
      std::cerr << " " << x.relpath << " (" << x.line_begin << "," << x.line_end
                << ")";
    std::cerr << std::endl;
#endif
  }

  std::string ResolveInclude(std::string from_relpath, std::string include) {
    std::string resolved;
    if (include[0] == '.' || files_data_.find(resolved) == files_data_.end()) {
      resolved = fs::path(from_relpath, fs::path::generic_format)
                     .remove_filename()
                     .append(include)
                     .lexically_normal()
                     .generic_u8string();
    } else {
      resolved = include;
    }
    if (files_data_.find(resolved) == files_data_.end()) resolved.clear();
#ifdef DEBUG
    if (!resolved.empty() && resolved != include) {
      std::cerr << "  Resolved " << include << " to " << resolved << " in "
                << from_relpath << " " << std::endl;
    }
#endif
    return resolved;
  }

  std::vector<fs::path> src_dirs_;
  std::vector<std::string> exts_;
  std::unordered_set<std::string> exclude_;
  bool inline_sources_;

  std::vector<std::string> files_;
  std::unordered_map<std::string, FileData> files_data_;
};

std::vector<std::string> StrSplit(const std::string &str, char sep) {
  std::vector<std::string> result;
  std::size_t end = 0;
  std::size_t pos = 0;
  while ((pos = str.find(sep, pos)) != std::string::npos) {
    result.push_back(str.substr(end, pos - end));
    ++pos;
    end = pos;
  }
  result.push_back(str.substr(end, str.size() - end));
  return result;
}

bool StartsWith(const std::string &str, const std::string &prefix,
                std::size_t pos = 0) {
  if (pos + prefix.size() > str.size()) return false;
  for (std::size_t i = 0; i < prefix.size(); ++i) {
    if (prefix[i] != str[i + pos]) return false;
  }
  return true;
}

bool ParseFlag(const std::string &arg, const std::string &name,
               std::unordered_map<std::string, std::string> *flags) {
  if (!StartsWith(arg, name, 2)) return false;
  if (arg[name.size() + 2] != '=') {
    throw std::runtime_error("Invalid argument: " + arg + arg[name.size() + 2]);
  }
  flags->emplace(name, arg.substr(name.size() + 3));
  return true;
}

}  // namespace

int main(int argc, char *argv[]) {
  fs::path root_dir;
  std::unordered_map<std::string, std::string> flags;
  static const auto *const kFlags =
      new std::vector<std::string>{"root", "exts", "out", "exclude", "inline"};
  for (int i = 1; i < argc; ++i) {
    const std::string &arg = argv[i];
    if (!StartsWith(arg, "--")) {
      throw std::runtime_error("Invalid argument (must start with --): " + arg);
    }
    for (const std::string &name : *kFlags) {
      if (ParseFlag(arg, name, &flags)) break;
    }
  }

  const auto &root_flag = flags.find("root");
  if (root_flag != flags.end() && !root_flag->second.empty()) {
    root_dir = root_flag->second;
  } else {
    root_dir = fs::current_path();
  }

  std::vector<std::string> exts;
  const auto &exts_flag = flags.find("exts");
  if (exts_flag != flags.end() && !exts_flag->second.empty()) {
    exts = StrSplit(exts_flag->second, ',');
  } else {
    exts = {".h", ".c", ".hpp", ".cpp"};
  }

  std::unordered_set<std::string> exclude;
  const auto &exclude_flag = flags.find("exclude");
  if (exclude_flag != flags.end()) {
    const auto vec = StrSplit(exclude_flag->second, ',');
    exclude = {vec.begin(), vec.end()};
  }

  const auto &inline_flag = flags.find("inline");
  bool inline_sources = false;
  if (inline_flag != flags.end() && inline_flag->second != "false") {
    if (inline_flag->second == "true") {
      inline_sources = true;
    } else {
      throw std::invalid_argument(
          "Invalid value for --inline. Expected true or false, got: " +
          inline_flag->second);
    }
  }

  Amalgamator amalgamator({root_dir.append("src")}, std::move(exts),
                          std::move(exclude), inline_sources);
  const auto &out_flag = flags.find("out");
  if (out_flag != flags.end() && !out_flag->second.empty()) {
    std::ofstream out(out_flag->second);
    amalgamator.Amalgamate(out);
  } else {
    amalgamator.Amalgamate(std::cout);
  }

  return 0;
}

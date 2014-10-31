#define SASS_SOURCE_MAP

#include <vector>

#ifndef SASS_MAPPING
#include "mapping.hpp"
#endif

#ifndef SASS_AST
#include "ast.hpp"
#endif

#ifndef SASSS_BASE64VLQ
#include "base64vlq.hpp"
#endif



namespace Sass {
  using std::vector;

  struct Context;

  class SourceMap {

  public:
    vector<size_t> source_index;
    SourceMap(const string& file);

    void remove_line();
    void update_position(const string& str);
    void add_mapping(AST_Node* node);

    string generate_source_map(Context &ctx);

  private:

    string serialize_mappings();

    vector<Mapping> mappings;
    Position output_position;
    string file;
    Base64VLQ base64vlq;
  };

}

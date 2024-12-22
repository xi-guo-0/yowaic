module;

#include "tree_sitter/api.h"

export module tree_sitter;

export {
  using ::ts_node_child;
  using ::ts_node_child_count;
  using ::ts_node_end_byte;
  using ::ts_node_is_null;
  using ::ts_node_start_byte;
  using ::ts_node_string;
  using ::ts_node_type;
  using ::ts_parser_delete;
  using ::ts_parser_new;
  using ::ts_parser_parse_string;
  using ::ts_parser_set_language;
  using ::ts_tree_delete;
  using ::ts_tree_root_node;
  using ::TSNode;
  using ::TSParser;
  using ::TSTree;
}

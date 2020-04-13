#ifndef NODE_H
#define NODE_H

namespace coom {
  extern const uint64_t NODE_MAX_INDEX;
  extern const uint64_t NODE_MAX_LABEL;

  uint64_t create_node_arc(uint64_t label, uint64_t index);
  uint64_t label_of(uint64_t n);
  uint64_t index_of(uint64_t n);
  bool is_node_arc(uint64_t n);

  uint64_t create_sink_arc(bool v);
  bool value_of(uint64_t n);
  bool is_sink_arc(uint64_t n);

  struct node
  {
    uint64_t data;
    uint64_t low;
    uint64_t high;
  };

  node create_node(uint64_t label, uint64_t index, uint64_t low, uint64_t high);
  uint64_t index_of(node n);
  uint64_t label_of(node n);
  bool operator< (const node& a, const node& b);
  bool operator> (const node& a, const node& b);
}

#endif // NODE_H

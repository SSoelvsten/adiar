#ifndef COOM_DATA_H
#define COOM_DATA_H

#include <stdint.h>

namespace coom {
  extern const uint64_t MAX_LABEL;
  extern const uint64_t MAX_ID;


  extern const uint64_t NIL;
  bool is_nil(uint64_t n);


  uint64_t create_node_ptr(uint64_t label, uint64_t index);
  uint64_t label_of(uint64_t n);
  uint64_t index_of(uint64_t n);
  bool is_node_ptr(uint64_t n);


  uint64_t create_sink(bool v);
  bool value_of(uint64_t n);
  bool is_sink(uint64_t n);


  struct node
  {
    uint64_t node_ptr;
    uint64_t low;
    uint64_t high;
  };

  node create_node(uint64_t label, uint64_t index, uint64_t low, uint64_t high);
  uint64_t index_of(const node& n);
  uint64_t label_of(const node& n);

  bool operator< (const node& a, const node& b);
  bool operator> (const node& a, const node& b);
  bool operator== (const node& a, const node& b);
  bool operator!= (const node& a, const node& b);

  struct arc
  {
    uint64_t source;
    bool is_high;
    uint64_t target;
  };

  inline arc create_arc(uint64_t source, bool is_high, uint64_t target);

  bool operator< (const arc& a, const arc& b);
  bool operator> (const arc& a, const arc& b);
  bool operator== (const arc& a, const arc& b);
  bool operator!= (const arc& a, const arc& b);

  arc low_arc_of_node(const node& n);
  arc high_arc_of_node(const node& n);
  node node_of_arcs(const arc& low, const arc& high);
}


#endif // COOM_DATA_H

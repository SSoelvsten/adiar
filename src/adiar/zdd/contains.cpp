#include "contains.h"

#include <adiar/file_stream.h>

#include <adiar/assert.h>

namespace adiar
{
  bool zdd_contains(const zdd &zdd, const label_file &labels)
  {
    node_stream<> ns(zdd);
    node_t n = ns.pull();

    label_stream<> ls(labels);

    if(is_sink(n)) {
      return value_of(n) && !ls.can_pull();
    }

    bool has_l = ls.can_pull();
    label_t l = has_l ? ls.pull() : 0;

    while (true) {
      while (ls.can_pull() && l < label_of(n)) {
        l = ls.pull();
      }

      const ptr_t next_ptr = has_l && l == label_of(n) ? n.high : n.low;

      // Found sink
      if(is_sink(next_ptr)) {
        return value_of(next_ptr)&& (!has_l || l <= label_of(n)) && !ls.can_pull();
      }

      // Check for missing a label
      if (has_l && l == label_of(n) && ls.can_pull()) {
        l = ls.pull();
      }

      if (label_of(n) < l && l < label_of(next_ptr)) {
        return false;
      }

      // "Recurse" to child
      while(n.uid < next_ptr) {
        adiar_debug(ns.can_pull(), "Invalid uid chasing; fell out of ZDD");
        n = ns.pull();
      }
    }
  }
}

#include "contains.h"

#include <adiar/data.h>
#include <adiar/file_stream.h>

#include <adiar/internal/traverse.h>

namespace adiar
{
  class zdd_contains_visitor
  {
    label_stream<> ls;

    bool has_l = false;
    label_t l;

    // First visit is for the root
    bool is_first_visit = true;

    // Remember what we saw the last time
    label_t visited_label;

    bool sink_val = false;

  public:
    zdd_contains_visitor(const label_file &labels) : ls(labels)
    {
      has_l = ls.can_pull();
      l = has_l ? ls.pull() : 0;
    }

    inline ptr_t visit(const node_t &n)
    {
      visited_label = label_of(n);

      const ptr_t next_ptr = has_l && l == visited_label ? n.high : n.low;

      if (has_l) {
        // Did we miss a label before the root?
        if (is_first_visit && l < visited_label) { return NIL; }

        // Will we miss a label?
        if (l == visited_label && ls.can_pull()) { l = ls.pull(); }
        if (is_node(next_ptr) && visited_label < l && l < label_of(next_ptr)) { return NIL; }
      }

      is_first_visit = false;
      return next_ptr;
    }

    inline void visit(const bool s)
    { sink_val = s; }

    inline bool get_result()
    { return sink_val && (!has_l || l <= visited_label) && !ls.can_pull(); }
  };

  bool zdd_contains(const zdd &zdd, const label_file &labels)
  {
    zdd_contains_visitor v(labels);
    traverse(zdd, v);
    return v.get_result();
  }
}

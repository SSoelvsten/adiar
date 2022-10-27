#include <adiar/zdd.h>

#include <adiar/label.h>
#include <adiar/file_stream.h>

#include <adiar/internal/algorithms/traverse.h>
#include <adiar/internal/data_types/node.h>

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

    bool terminal_val = false;

  public:
    zdd_contains_visitor(const label_file &labels) : ls(labels)
    {
      has_l = ls.can_pull();
      l = has_l ? ls.pull() : 0;
    }

    inline ptr_uint64 visit(const node_t &n)
    {
      visited_label = n.label();

      const ptr_uint64 next_ptr = has_l && l == visited_label ? n.high() : n.low();

      if (has_l) {
        // Did we miss a label before the root?
        if (is_first_visit && l < visited_label) { return ptr_uint64::NIL(); }

        // Will we miss a label?
        if (l == visited_label && ls.can_pull()) { l = ls.pull(); }
        if (next_ptr.is_node() && visited_label < l && l < next_ptr.label()) {
          return ptr_uint64::NIL();
        }
      }

      is_first_visit = false;
      return next_ptr;
    }

    inline void visit(const bool s)
    { terminal_val = s; }

    inline bool get_result()
    { return terminal_val && (!has_l || l <= visited_label) && !ls.can_pull(); }
  };

  bool zdd_contains(const zdd &zdd, const label_file &labels)
  {
    zdd_contains_visitor v(labels);
    traverse(zdd, v);
    return v.get_result();
  }
}

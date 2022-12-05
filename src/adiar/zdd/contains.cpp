#include <adiar/zdd.h>

#include <adiar/internal/algorithms/traverse.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file_stream.h>

namespace adiar
{
  class zdd_contains_visitor
  {
    internal::file_stream<zdd::label_t> ls;

    bool has_l = false;
    zdd::label_t l;

    // First visit is for the root
    bool is_first_visit = true;

    // Remember what we saw the last time
    zdd::label_t visited_label;

    bool terminal_val = false;

  public:
    zdd_contains_visitor(const shared_file<zdd::label_t> &labels) : ls(labels)
    {
      has_l = ls.can_pull();
      l = has_l ? ls.pull() : 0;
    }

    inline zdd::ptr_t visit(const zdd::node_t &n)
    {
      visited_label = n.label();

      const zdd::ptr_t next_ptr = has_l && l == visited_label ? n.high() : n.low();

      if (has_l) {
        // Did we miss a label before the root?
        if (is_first_visit && l < visited_label) { return zdd::ptr_t::NIL(); }

        // Will we miss a label?
        if (l == visited_label && ls.can_pull()) { l = ls.pull(); }
        if (next_ptr.is_node() && visited_label < l && l < next_ptr.label()) {
          return zdd::ptr_t::NIL();
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

  bool zdd_contains(const zdd &zdd, const shared_file<zdd::label_t> &labels)
  {
    zdd_contains_visitor v(labels);
    internal::traverse(zdd, v);
    return v.get_result();
  }
}

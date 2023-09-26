#include <adiar/zdd.h>

#include <adiar/internal/algorithms/traverse.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file_stream.h>

namespace adiar
{
  class zdd_contains_visitor
  {
    const generator<zdd::label_t> &gen;

    bool has_l = false;
    zdd::label_t l;

    // First visit is for the root
    bool is_first_visit = true;

    // Remember what we saw the last time
    zdd::label_t visited_label;

    bool terminal_val = false;

  public:
    zdd_contains_visitor(const generator<zdd::label_t> &a)
      : gen(a)
    {
      l = gen();
      has_l = l <= zdd::max_label;
    }

    inline zdd::ptr_t visit(const zdd::node_t &n)
    {
      visited_label = n.label();

      const zdd::ptr_t next_ptr = has_l && l == visited_label ? n.high() : n.low();

      if (has_l) {
        // Did we miss a label before the root?
        if (is_first_visit && l < visited_label) { return zdd::ptr_t::nil(); }

        // Will we miss a label?
        if (l == visited_label && l <= zdd::max_label) { l = gen(); }
        if (next_ptr.is_node() && visited_label < l && l < next_ptr.label()) {
          return zdd::ptr_t::nil();
        }
      }

      is_first_visit = false;
      return next_ptr;
    }

    inline void visit(const bool s)
    { terminal_val = s; }

    inline bool get_result()
    { return terminal_val && zdd::max_label < l; }
  };

  bool zdd_contains(const zdd &zdd, const generator<zdd::label_t> &a)
  {
    zdd_contains_visitor v(a);
    internal::traverse(zdd, v);
    return v.get_result();
  }
}

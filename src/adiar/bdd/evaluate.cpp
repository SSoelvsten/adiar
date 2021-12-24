#include "evaluate.h"

#include <adiar/file_stream.h>

#include <adiar/assert.h>

namespace adiar
{
  bool bdd_eval(const bdd &bdd, const assignment_func &af)
  {
    node_stream<> ns(bdd);
    node_t current_node = ns.pull();

    if(is_sink(current_node)) {
      return value_of(current_node);
    }

    while (true) {
      ptr_t next_ptr = unflag(af(label_of(current_node)) ? current_node.high : current_node.low);

      if(is_sink(next_ptr)) {
        return value_of(next_ptr);
      }

      while(current_node.uid < next_ptr) {
        adiar_debug(ns.can_pull(), "Invalid uid chasing; fell out of BDD");
        current_node = ns.pull();
      }
    }
  }

  bool bdd_eval(const bdd &bdd, const assignment_file &assignments)
  {
    if (is_sink(bdd)) {
      return is_sink(bdd, is_true);
    }

    assignment_stream<> as(assignments);
    assignment_t a = as.pull();

    const assignment_func af = [&as, &a](label_t l)
    {
      while (l > label_of(a)) {
        adiar_assert(as.can_pull(), "Given assignment file is insufficient to traverse BDD");
        a = as.pull();
      }

      adiar_assert(l == label_of(a), "Missing assignment for node visited in BDD");

      return value_of(a);
    };

    return bdd_eval(bdd, af);
  }
}

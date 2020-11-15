#ifndef COOM_EVALUATE_CPP
#define COOM_EVALUATE_CPP

#include "evaluate.h"

#include <coom/assert.h>

namespace coom
{
  bool bdd_eval(const node_file &nodes, const assignment_file &assignments)
  {
    node_stream<> ns(nodes);
    node_t current_node = ns.pull();

    if(is_sink(current_node)) {
      return value_of(current_node);
    }

    assignment_stream<> as(assignments);
    assignment_t a = as.pull();

    while (true) {
      while(label_of(current_node) > a.label) {
        a = as.pull();
      }
      coom_assert(label_of(current_node) == a.label,
                  "Missing assignment for node visited in BDD");

      ptr_t next_ptr = unflag(a.value ? current_node.high : current_node.low);

      if(is_sink_ptr(next_ptr)) {
        return value_of(next_ptr);
      }

      while(current_node.uid < next_ptr) {
#if COOM_ASSERT
        assert(ns.can_pull());
#endif
        current_node = ns.pull();
      }
    }
  }
}

#endif // COOM_EVALUATE_CPP

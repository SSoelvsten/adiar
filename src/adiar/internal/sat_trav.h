#ifndef ADIAR_INTERNAL_SAT_TRAV_H
#define ADIAR_INTERNAL_SAT_TRAV_H

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>

namespace adiar
{
  typedef std::function<void(label_t, bool)> sat_trav_callback_t;

  template<typename sat_trav_policy>
  void sat_trav(const typename sat_trav_policy::reduced_t &dd,
                const sat_trav_callback_t &callback)
  {
    node_stream<> in_nodes(dd);
    node_t n_curr = in_nodes.pull();
    ptr_t  n_next = n_curr.uid;

    while (!is_sink(n_next)) {
      // forward to n_next
      while (n_curr.uid < n_next) { n_curr = in_nodes.pull(); }

      adiar_debug(n_curr.uid == n_next,
                  "Invalid uid chasing; fell out of Decision Diagram");

      const bool go_high = sat_trav_policy::go_high(n_curr);
      callback(label_of(n_curr), go_high);

      n_next = go_high ? n_curr.high : n_curr.low;
    }
  }
}

#endif // ADIAR_INTERNAL_SAT_TRAV_H

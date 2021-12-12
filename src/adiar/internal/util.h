#ifndef ADIAR_INTERNAL_UTIL_H
#define ADIAR_INTERNAL_UTIL_H

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>

#include <adiar/internal/decision_diagram.h>

namespace adiar {
  template<typename in1_t = decision_diagram, typename stream1_t = level_info_stream<node_t, NODE_FILE_COUNT>,
           typename in2_t = decision_diagram, typename stream2_t = level_info_stream<node_t, NODE_FILE_COUNT>>
  bool disjoint_labels(const in1_t &in1, const in2_t &in2)
  {
    stream1_t s1(in1);
    stream2_t s2(in2);

    while(s1.can_pull() && s2.can_pull()) {
      if (label_of(s1.peek()) == label_of(s2.peek())) {
        return false;
      } else if (label_of(s1.peek()) < label_of(s2.peek())) {
        s1.pull();
      } else {
        s2.pull();
      }
    }
    return true;
  }
}

#endif // ADIAR_INTENRAL_UTIL_H

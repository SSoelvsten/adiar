#include "decision_diagram.h"

#include <adiar/data.h>
#include <adiar/file_stream.h>

namespace adiar {
  __decision_diagram::__decision_diagram(const decision_diagram &dd)
    : _union(dd.file), negate(dd.negate) { };

  //////////////////////////////////////////////////////////////////////////////
  bool is_canonical(const decision_diagram &dd)
  {
    return dd.file._file_ptr -> canonical;
  }

  bool is_sink(const decision_diagram &dd, const sink_pred &pred)
  {
    node_stream<> ns(dd);
    node_t n = ns.pull();

    return is_sink(n) && pred(n.uid);
  }

  label_t min_label(const decision_diagram &dd)
  {
    return min_label(dd.file);
  }

  label_t max_label(const decision_diagram &dd)
  {
    return max_label(dd.file);
  }
}

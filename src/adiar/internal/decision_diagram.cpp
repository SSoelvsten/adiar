#include "decision_diagram.h"

#include <adiar/data.h>
#include <adiar/file_stream.h>

namespace adiar {
  __decision_diagram::__decision_diagram(const decision_diagram &dd)
    : _union(dd.file), negate(dd.negate) { };

  //////////////////////////////////////////////////////////////////////////////
  bool is_canonical(const decision_diagram &dd)
  {
    return dd.file_ptr() -> canonical;
  }

  bool is_sink(const decision_diagram &dd)
  {
    return is_sink(dd.file);
  }

  bool value_of(const decision_diagram &dd)
  {
    return dd.negate ^ value_of(dd.file);
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

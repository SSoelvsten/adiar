#include "convert.h"

#include <adiar/internal/convert.h>

namespace adiar
{
  __zdd zdd_from(const bdd &dd, const label_file &dom)
  {
    return intercut<convert_decision_diagram_policy<zdd_policy, bdd_policy>>(dd,dom);
  }

  __bdd bdd_from(const zdd &dd, const label_file &dom)
  {
    return intercut<convert_decision_diagram_policy<bdd_policy, zdd_policy>>(dd,dom);
  }
}

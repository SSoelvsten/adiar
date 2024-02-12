#include <adiar/bdd.h>

#include <adiar/internal/algorithms/dot.h>

namespace adiar
{
  void
  bdd_printdot(const bdd& f, std::ostream& out, bool include_id)
  {
    internal::print_dot<bdd>(f, out, include_id);
  }

  void
  bdd_printdot(const bdd& f, const std::string& file_name, bool include_id)
  {
    internal::print_dot<bdd>(f, file_name, include_id);
  }
}

#include <adiar/zdd.h>

#include <adiar/internal/algorithms/dot.h>

namespace adiar
{
  void
  zdd_printdot(const zdd& A, std::ostream& out, bool include_id)
  {
    internal::print_dot<zdd>(A, out, include_id);
  }

  void
  zdd_printdot(const zdd& A, const std::string& file_name, bool include_id)
  {
    internal::print_dot<zdd>(A, file_name, include_id);
  }
}

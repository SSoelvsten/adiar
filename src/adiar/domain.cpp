#include "domain.h"

#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/shared_file_ptr.h>

namespace adiar
{
  // Use an explicit 'shared_ptr' rather than a 'shared_file' to allow using the
  // NULL value.

  shared_ptr<internal::file<domain_var>> domain_ptr;

  void domain_set(const domain_var varcount) {
    shared_file<domain_var> dom;
    { // Garbage collect writer to free write-lock
      internal::file_writer<domain_var> lw(dom);
      for (domain_var v = 0; v < varcount; v++) { lw << v; }
    }
    domain_set(dom);
  }

  void domain_set(const generator<domain_var> &gen) {
    shared_file<domain_var> dom;
    { // Garbage collect writer to free write-lock
      internal::file_writer<domain_var> lw(dom);

      domain_var v;
      while ((v = gen()) <= domain_max) { lw << v; }
    }

    domain_set(dom);
  }

  void domain_set(const shared_file<domain_var> &dom) {
    domain_ptr = dom;
  }

  void domain_unset() {
    domain_ptr.reset();
  }

  bool domain_isset() {
    return domain_ptr ? true : false;
  }

  shared_file<domain_var> domain_get() {
    if(!domain_isset()) {
      throw domain_error("Domain must be set before it can be used");
    }

    return domain_ptr;
  }
}

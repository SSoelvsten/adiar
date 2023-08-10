#include "domain.h"

#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/shared_file_ptr.h>

namespace adiar
{
  // Use an explicit 'shared_ptr' rather than a 'shared_file' to allow using the
  // NULL value.

  shared_ptr<internal::file<domain_var_t>> domain_ptr;

  void adiar_set_domain(const domain_var_t varcount) {
    shared_file<domain_var_t> dom;
    { // Garbage collect writer to free write-lock
      internal::file_writer<domain_var_t> lw(dom);
      for (domain_var_t v = 0; v < varcount; v++) { lw << v; }
    }
    adiar_set_domain(dom);
  }

  void adiar_set_domain(const std::function<domain_var_t()> &gen) {
    shared_file<domain_var_t> dom;
    { // Garbage collect writer to free write-lock
      internal::file_writer<domain_var_t> lw(dom);

      domain_var_t v;
      while ((v = gen()) <= MAX_DOMAIN_VAR) { lw << v; }
    }

    adiar_set_domain(dom);
  }

  void adiar_set_domain(const shared_file<domain_var_t> &dom) {
    domain_ptr = dom;
  }

  void adiar_unset_domain() {
    domain_ptr.reset();
  }

  bool adiar_has_domain() {
    return domain_ptr ? true : false;
  }

  shared_file<domain_var_t> adiar_get_domain() {
    if(!adiar_has_domain()) {
      throw domain_error("Domain must be set before it can be used");
    }

    return domain_ptr;
  }
}

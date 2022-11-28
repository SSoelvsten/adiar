#include "domain.h"

#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/shared_file.h>

namespace adiar
{
  shared_ptr<internal::file<internal::node::label_t>> domain_ptr;

  void adiar_set_domain(const internal::label_file &dom) {
    domain_ptr = dom;
  }

  bool adiar_has_domain() {
    return domain_ptr ? true : false;
  }

  internal::label_file adiar_get_domain() {
    if(!adiar_has_domain()) {
      throw std::domain_error("Domain must be set before it can be used");
    }

    return domain_ptr;
  }
}

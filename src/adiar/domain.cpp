#include "domain.h"

#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/shared_file_ptr.h>

namespace adiar
{
  // Use an explicit 'shared_ptr' rather than a 'shared_file' to allow using the
  // NULL value.

  shared_ptr<internal::file<internal::node::label_t>> domain_ptr;

  void adiar_set_domain(const shared_file<internal::node::label_t> &dom) {
    domain_ptr = dom;
  }

  bool adiar_has_domain() {
    return domain_ptr ? true : false;
  }

  shared_file<internal::node::label_t> adiar_get_domain() {
    if(!adiar_has_domain()) {
      throw std::domain_error("Domain must be set before it can be used");
    }

    return domain_ptr;
  }
}

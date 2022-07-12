#include "domain.h"

namespace adiar
{
  std::shared_ptr<label_file> domain_ptr;

  void adiar_set_domain(const label_file &dom) {
    domain_ptr = std::make_shared<label_file>(dom);
  }

  bool adiar_has_domain() {
    return domain_ptr ? true : false;
  }

  label_file adiar_get_domain() {
    if(!adiar_has_domain()) {
      throw std::domain_error("Domain must be set before it can be used");
    }

    return *(domain_ptr.get());
  }
}

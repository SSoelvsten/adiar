#include "build.h"

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/build.h>

namespace adiar
{
  zdd zdd_sink(bool value)
  {
    return build_sink(value);
  }

  zdd zdd_empty()
  {
    return build_sink(false);
  }

  zdd zdd_null()
  {
    return build_sink(true);
  }

  zdd zdd_ithvar(label_t label)
  {
    return build_ithvar(label);
  }

  zdd zdd_vars(const label_file &labels)
  {
    return build_chain<true, false, true>(labels);
  }

  zdd zdd_singletons(const label_file &labels)
  {
    return build_chain<false, true, false>(labels);
  }

  zdd zdd_powerset(const label_file &labels)
  {
    return build_chain<true, true, true, true>(labels);
  }
}

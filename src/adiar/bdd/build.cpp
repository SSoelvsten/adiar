#include <adiar/bdd.h>

#include <adiar/bdd/bdd_policy.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/levelized_file_writer.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/algorithms/build.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/data_types/ptr.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_terminal(bool value)
  {
    return internal::build_terminal<bdd_policy>(value);
  }

  bdd bdd_true()
  {
    return internal::build_terminal<bdd_policy>(true);
  }

  bdd bdd_false()
  {
    return internal::build_terminal<bdd_policy>(false);
  }

  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_ithvar(bdd::label_t label)
  {
    return internal::build_ithvar<bdd_policy>(label);
  }

  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_nithvar(bdd::label_t label)
  {
    return bdd_not(internal::build_ithvar<bdd_policy>(label));
  }

  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_and(const generator<bdd::label_t> &vars)
  {
    internal::chain_high<bdd_policy> p;
    return internal::build_chain<>(p, vars);
  }

  //////////////////////////////////////////////////////////////////////////////
  bdd bdd_or(const generator<bdd::label_t> &vars)
  {
    internal::chain_low<bdd_policy> p;
    return internal::build_chain<>(p, vars);
  }
}

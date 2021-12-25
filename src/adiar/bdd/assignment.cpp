#include "assignment.h"

#include <adiar/data.h>

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/sat_trav.h>

namespace adiar
{
  template<typename policy>
  inline assignment_file bdd_satX(const bdd &f)
  {
    assignment_file out;
    assignment_writer aw(out);

    level_info_stream<node_t,1> ms(f);

    const sat_trav_callback_t callback = [&aw, &ms](const label_t label, const bool value) {
      // set default to all skipped levels
      while (label_of(ms.peek()) < label) {
        aw << create_assignment(label_of(ms.pull()), policy::skipped_value);
      }
      adiar_debug(label_of(ms.peek()) == label,
                  "level given should exist in BDD");

      aw << create_assignment(label_of(ms.pull()), value);
    };

    sat_trav<policy>(f, callback);

    while (ms.can_pull()) {
      aw << create_assignment(label_of(ms.pull()), policy::skipped_value);
    }

    return out;
  }

  class bdd_satmin_policy : public bdd_policy, public sat_trav_min_policy
  {
  public:
    static constexpr bool skipped_value = false;
  };

  assignment_file bdd_satmin(const bdd &f)
  {

    return bdd_satX<bdd_satmin_policy>(f);
  }

  class bdd_satmax_policy : public bdd_policy, public sat_trav_max_policy
  {
  public:
    static constexpr bool skipped_value = true;
  };

  assignment_file bdd_satmax(const bdd &f)
  {
    return bdd_satX<bdd_satmax_policy>(f);
  }
}

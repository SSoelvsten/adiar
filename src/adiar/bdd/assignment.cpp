#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/sat_trav.h>

namespace adiar
{
  template<typename policy>
  inline assignment_file bdd_satX(const bdd &f)
  {
    adiar_assert(!is_sink(f),
                 "Cannot extract an assignment from a sink file");

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

  class bdd_satmin_policy : public bdd_policy
  {
  public:
    static constexpr bool skipped_value = false;

    static bool go_high(const node_t &n) {
      // Only pick high, if low is the false sink
      return is_sink(n.low) && !value_of(n.low);
    }
  };

  assignment_file bdd_satmin(const bdd &f)
  {

    return bdd_satX<bdd_satmin_policy>(f);
  }

  class bdd_satmax_policy : public bdd_policy
  {
  public:
    static constexpr bool skipped_value = true;

    static bool go_high(const node_t &n) {
      // Pick high as long it is not the false sink
      return is_node(n.high) || value_of(n.high);
    }
  };

  assignment_file bdd_satmax(const bdd &f)
  {
    return bdd_satX<bdd_satmax_policy>(f);
  }
}

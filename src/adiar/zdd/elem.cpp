#include "elem.h"

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/sat_trav.h>

#include <adiar/zdd/zdd.h>

namespace adiar
{
  template<typename policy>
  inline std::optional<label_file> zdd_Xelem(const zdd &A)
  {
    if (is_sink(A, is_false)) {
      return std::nullopt;
    }

    label_file out;
    label_writer lw(out);

    const sat_trav_callback_t callback = [&lw](const label_t label, const bool value) {
      if (value) { lw << label; }
    };

    sat_trav<policy>(A, callback);

    return out;
  }

  class zdd_minelem_policy : public zdd_policy, public sat_trav_min_policy
  { };

  std::optional<label_file> zdd_minelem(const zdd &A)
  {
    return zdd_Xelem<zdd_minelem_policy>(A);
  }

  class zdd_maxelem_policy : public zdd_policy
  {
  public:
    inline static bool go_high(const node_t &n) {
      adiar_debug(!is_sink(n.high) || value_of(n.high), "high sinks are never false");
      return true;
    }
  };

  std::optional<label_file> zdd_maxelem(const zdd &A)
  {
    return zdd_Xelem<zdd_maxelem_policy>(A);
  }
}

#include "adiar/exec_policy.h"
#include "adiar/functional.h"
#include "adiar/internal/data_types/tuple.h"
#include "adiar/internal/io/node_writer.h"
#include "adiar/internal/io/shared_file_ptr.h"
#include "adiar/types.h"
#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>
#include <adiar/internal/algorithms/optmin.h>

namespace adiar
{

  // TODO: Allow for a tiebreak on multiple equally costly paths? This could allow for prioritizing
  // fx the solution with fewest valiables set. It is not obvious whether there are cases of this
  // which couldn't be accomplished by modifying the costs of the nodes. It is also not obvious
  // what the interface should look like.
  //
  // Maybe this should be compatible with bullying, in which case bullying needs to not bully
  // requests which are equally good as our current min.
  class bdd_optmin_policy : public bdd_policy
  {
  public:
    static constexpr bool bullying = true;
    void
    out(bdd::label_type label, bool value)
    {
      this->out_fn({label, value});
    }

    double
    cost_fn(bdd::label_type label)
    {
      auto cost = get_cost(label);
      if (cost < 0.0) {
        throw adiar::domain_error("BDDs does not support negative cost functions");
      }
      return cost;
    };

    bdd_optmin_policy(const consumer<pair<bdd::label_type, bool>>& o,
                      const cost<bdd::label_type>& c)
        : out_fn(o), get_cost(c)
    {}

  private:
    const consumer<pair<bdd::label_type, bool>>& out_fn;
    const cost<bdd::label_type>& get_cost;
  };

  double
  bdd_optmin(const exec_policy& ep,
             const bdd& f,
             const cost<bdd::label_type>& c,
             const consumer<pair<bdd::label_type, bool>>& cb)
  {
    const bdd_optmin_policy omp(cb, c);
    return internal::optmin<bdd_optmin_policy>(ep, omp, f);
  }

  double
  bdd_optmin(const bdd& f,
             const cost<bdd::label_type>& c,
             const consumer<pair<bdd::label_type, bool>>& cb)
  {
    return bdd_optmin(exec_policy(), f, c, cb);
  }

  pair<bdd, double>
  bdd_optmin(const exec_policy& ep, const bdd& f, const cost<bdd::label_type>& c)
  {
    bdd::shared_node_file_type nf;

    if (f->is_terminal()) {
      // As f is already the cube we want to return, we'll just return that
      return pair<bdd, double>(f, bdd_optmin(ep, f, c, [](pair<bdd::label_type, bool>) {}));
    }

    internal::node_writer nw(nf);
    bdd::pointer_type root = bdd::pointer_type(true);
    double value           = bdd_optmin(ep, f, c, [&nw, &root](pair<bdd::label_type, bool> pair) {
      const auto [lt, value] = pair;
      bdd::node_type next(lt,
                          bdd::max_id,
                          value ? bdd::pointer_type(false) : root,
                          value ? root : bdd::pointer_type(false));
      nw << next;
      root = next.uid();
    });

    return {nf, value};
  }

  pair<bdd, double>
  bdd_optmin(const bdd& f, const cost<bdd::label_type>& c)
  {
    return bdd_optmin(exec_policy(), f, c);
  }
} // namespace adiar

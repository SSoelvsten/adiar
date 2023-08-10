#ifndef ADIAR_INTERNAL_ALGORITHMS_CONVERT_H
#define ADIAR_INTERNAL_ALGORITHMS_CONVERT_H

#include <adiar/exception.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/unreachable.h>
#include <adiar/internal/algorithms/intercut.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/tuple.h>
#include <adiar/internal/io/node_file.h>
#include <adiar/internal/io/node_writer.h>

namespace adiar::internal
{
  template<typename to_policy, typename from_policy>
  class convert_dd_policy
  {
    static_assert(std::is_base_of<dd,   typename from_policy::reduced_t>::value);
    static_assert(std::is_base_of<__dd, typename from_policy::unreduced_t>::value);
    static_assert(std::is_base_of<dd,   typename to_policy::reduced_t>::value);
    static_assert(std::is_base_of<__dd, typename to_policy::unreduced_t>::value);

  public:
    typedef typename from_policy::reduced_t reduced_t;
    typedef typename from_policy::ptr_t ptr_t;
    typedef typename from_policy::node_t node_t;

    typedef typename to_policy::unreduced_t unreduced_t;
    typedef typename to_policy::label_t label_t;
    typedef typename to_policy::id_t id_t;

    static constexpr typename to_policy::label_t MAX_LABEL = to_policy::MAX_LABEL;
    static constexpr typename to_policy::id_t MAX_ID = to_policy::MAX_ID;

  public:
    static constexpr bool may_skip = true;

    static constexpr bool cut_true_terminal  = true;
    static constexpr bool cut_false_terminal = false;

    static constexpr size_t mult_factor = 2u;

  public:
    static typename to_policy::reduced_t
    on_empty_labels(const typename from_policy::reduced_t& dd)
    {
      if (!is_terminal(dd)) {
        throw invalid_argument("Only a pure terminal can be part of an empty domain");
      }
      return typename to_policy::reduced_t(dd.file, dd.negate);
    }

    static typename to_policy::reduced_t
    on_terminal_input(const bool terminal_value,
                      const typename from_policy::reduced_t& /*dd*/,
                      const shared_file<typename from_policy::label_t> &dom)
    {
      adiar_assert(dom->size() > 0, "Emptiness check is before terminal check");

      node::uid_t prior_node = node::uid_t(terminal_value);

      shared_levelized_file<bdd::node_t> nf;

      bool has_output = true;
      node_writer nw(nf);

      file_stream<typename from_policy::label_t, true> ls(dom);

      while(ls.can_pull()) {
        const typename to_policy::label_t next_label = ls.pull();

        if (to_policy::MAX_LABEL < next_label) {
          throw invalid_argument("Domain contains unrepresentable labels");
        }
        if (!prior_node.is_terminal() && prior_node.label() <= next_label) {
          throw invalid_argument("Labels are not provided in increasing order");
        }

        const tuple children = from_policy::reduction_rule_inv(prior_node);
        const node next_node = node(next_label, to_policy::MAX_ID, children[0], children[1]);
        const typename to_policy::ptr_t reduction_result = to_policy::reduction_rule(next_node);

        if (reduction_result == next_node.uid()) { // Output
          prior_node = next_node.uid();

          nw.unsafe_push(next_node);
          nw.unsafe_push(level_info(next_label,1u));
        } else {
          // If we kill the resulting node once, then we will also do it for all
          // the other labels we still are missing.
          has_output = false;
          break;
        }
      }

      if (!has_output) {
        nw.unsafe_push(node(terminal_value));
      }
      return nf;
    }

    static typename to_policy::reduced_t
    terminal(const bool terminal_value)
    {
      // Notice, that both bdd_t and zdd_t have bool constructors
      return terminal_value;
    }

    static intercut_rec hit_existing(const node &n)
    {
      const node::ptr_t to_reduction = to_policy::reduction_rule(n);
      if (to_reduction != n.uid()) {
        return intercut_rec_skipto { to_reduction };
      }
      return intercut_rec_output { n.low(), n.high() };
    }

    static intercut_rec_output hit_cut(const node::ptr_t target)
   {
      const tuple children = from_policy::reduction_rule_inv(target);

      // Debug mode: double-check we don't create irrelevant nodes
      adiar_assert(to_policy::reduction_rule(node(0,0, children[0], children[1])) != target,
                   "Should not cut an arc where the one created will be killed anyways.");

      return intercut_rec_output { children[0], children[1] };
    }

    // LCOV_EXCL_START
    static intercut_rec miss_existing(const node &/*n*/)
    { adiar_unreachable(); }
    // LCOV_EXCL_STOP
  };
}

#endif // ADIAR_INTERNAL_ALGORITHMS_CONVERT_H

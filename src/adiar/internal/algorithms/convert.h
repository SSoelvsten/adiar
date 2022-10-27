#ifndef ADIAR_INTERNAL_ALGORITHMS_CONVERT_H
#define ADIAR_INTERNAL_ALGORITHMS_CONVERT_H

#include<adiar/label.h>

#include<adiar/internal/decision_diagram.h>

#include<adiar/internal/algorithms/intercut.h>

#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/tuple.h>

namespace adiar
{
  template<typename to_policy, typename from_policy>
  class convert_decision_diagram_policy
  {
    static_assert(std::is_base_of<decision_diagram,   typename from_policy::reduced_t>::value);
    static_assert(std::is_base_of<__decision_diagram, typename from_policy::unreduced_t>::value);
    static_assert(std::is_base_of<decision_diagram,   typename to_policy::reduced_t>::value);
    static_assert(std::is_base_of<__decision_diagram, typename to_policy::unreduced_t>::value);

  public:
    typedef typename from_policy::reduced_t reduced_t;
    typedef typename to_policy::unreduced_t unreduced_t;

  public:
    static constexpr bool may_skip = true;

    static constexpr bool cut_true_terminal  = true;
    static constexpr bool cut_false_terminal = false;

    static constexpr size_t mult_factor = 2u;

  public:
    static typename to_policy::reduced_t
    on_empty_labels(const typename from_policy::reduced_t& dd)
    {
      adiar_assert(is_terminal(dd), "Only a pure terminal can be part of an empty domain");
      return typename to_policy::reduced_t(dd.file, dd.negate);
    }

    static typename to_policy::reduced_t
    on_terminal_input(const bool terminal_value,
                  const typename from_policy::reduced_t& /*dd*/,
                  const label_file &dom)
    {
      adiar_debug(dom.size() > 0, "Emptiness check is before terminal check");

      ptr_uint64 prior_node = ptr_uint64(terminal_value);

      node_file nf;

      bool has_output = true;
      node_writer nw(nf);

      label_stream<true> ls(dom);

      while(ls.can_pull()) {
        const label_t next_label = ls.pull();

        adiar_assert(next_label <= MAX_LABEL, "Cannot represent that large a label");
        adiar_assert(prior_node.is_terminal() || next_label < prior_node.label(),
                     "Labels not given in increasing order");

        const tuple children = from_policy::reduction_rule_inv(prior_node);
        const node_t next_node = node(next_label, MAX_ID, children.t1, children.t2);
        const ptr_uint64 reduction_result = to_policy::reduction_rule(next_node);

        if (reduction_result == next_node.uid()) { // Output
          prior_node = next_node.uid();

          nw.unsafe_push(next_node);
          nw.unsafe_push(create_level_info(next_label,1u));
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

    static intercut_rec hit_existing(const node_t &n)
    {
      const ptr_uint64 to_reduction = to_policy::reduction_rule(n);
      if (to_reduction != n.uid()) {
        return intercut_rec_skipto { to_reduction };
      }
      return intercut_rec_output { n.low(), n.high() };
    }

    static intercut_rec_output hit_cut(const ptr_uint64 target)
   {
      const tuple children = from_policy::reduction_rule_inv(target);

      // Debug mode: double-check we don't create irrelevant nodes
      adiar_debug(to_policy::reduction_rule(node(0,0, children.t1, children.t2)) != target,
                  "Should not cut an arc where the one created will be killed anyways.");

      return intercut_rec_output { children.t1, children.t2 };
    }

    // LCOV_EXCL_START
    static intercut_rec miss_existing(const node_t &/*n*/)
    { adiar_unreachable(); }
    // LCOV_EXCL_END
  };
}

#endif // ADIAR_INTERNAL_ALGORITHMS_CONVERT_H

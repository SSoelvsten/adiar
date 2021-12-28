#ifndef ADIAR_INTERNAL_CONVERT_H
#define ADIAR_INTERNAL_CONVERT_H

#include<adiar/data.h>

#include<adiar/internal/decision_diagram.h>
#include<adiar/internal/intercut.h>

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

    static constexpr bool cut_true_sink  = true;
    static constexpr bool cut_false_sink = false;

  public:
    static typename to_policy::reduced_t
    on_empty_labels(const typename from_policy::reduced_t& dd)
    {
      adiar_assert(is_sink(dd), "Only a pure sink can be part of an empty domain");
      return typename to_policy::reduced_t(dd.file, dd.negate);
    }

    static typename to_policy::reduced_t
    on_sink_input(const bool sink_value,
                  const typename from_policy::reduced_t& /*dd*/,
                  const label_file &dom)
    {
      adiar_debug(dom.size() > 0, "Emptiness check is before sink check");

      ptr_t prior_node = create_sink_ptr(sink_value);

      node_file nf;

      bool has_output = true;
      node_writer nw(nf);

      label_stream<true> ls(dom);

      while(ls.can_pull()) {
        const label_t next_label = ls.pull();

        adiar_assert(next_label <= MAX_LABEL, "Cannot represent that large a label");
        adiar_assert(is_sink(prior_node) || next_label < label_of(prior_node),
                     "Labels not given in increasing order");

        const tuple children = from_policy::reduction_rule_inv(prior_node);
        const node_t next_node = create_node(next_label, MAX_ID, children.t1, children.t2);
        const ptr_t reduction_result = to_policy::reduction_rule(next_node);

        if (reduction_result == next_node.uid) { // Output
          prior_node = next_node.uid;

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
        nw.unsafe_push(create_sink(sink_value));
      }
      return nf;
    }

    static typename to_policy::reduced_t
    sink(const bool sink_value)
    {
      // Notice, that both bdd_t and zdd_t have bool constructors
      return sink_value;
    }

    static intercut_rec hit_existing(const node_t &n)
    {
      const ptr_t to_reduction = to_policy::reduction_rule(n);
      if (to_reduction != n.uid) {
        return intercut_rec_skipto { to_reduction };
      }
      return intercut_rec_output { n.low, n.high };
    }

    static intercut_rec_output hit_cut(const ptr_t target)
   {
      const tuple children = from_policy::reduction_rule_inv(target);

      // Debug mode: double-check we don't create irrelevant nodes
      adiar_debug(to_policy::reduction_rule(create_node(0,0, children.t1, children.t2)) != target,
                  "Should not cut an arc where the one created will be killed anyways.");

      return intercut_rec_output { children.t1, children.t2 };
    }

    // LCOV_EXCL_START
    static intercut_rec miss_existing(const node_t &/*n*/)
    { adiar_unreachable(); }
    // LCOV_EXCL_END
  };
}

#endif // ADIAR_INTERNAL_CONVERT_H

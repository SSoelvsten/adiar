#ifndef ADIAR_INTERNAL_ALGORITHMS_BUILD_H
#define ADIAR_INTERNAL_ALGORITHMS_BUILD_H

#include <adiar/exception.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/node_writer.h>
#include <adiar/internal/io/shared_file_ptr.h>

namespace adiar::internal
{
  template<typename dd_policy>
  inline
  shared_levelized_file<typename dd_policy::node_type>
  build_terminal(bool value)
  {
    using node_type = typename dd_policy::node_type;
    shared_levelized_file<node_type> nf;
    {
      node_writer nw(nf);
      nw.unsafe_push(node_type(value));
      nw.unsafe_set_number_of_terminals(!value, value);
      nw.unsafe_set_canonical(true);
    }

    return nf;
  }

  template<typename dd_policy>
  inline
  shared_levelized_file<typename dd_policy::node_type>
  build_ithvar(typename dd_policy::label_type label)
  {
    using node_type = typename dd_policy::node_type;
    using pointer_type = typename node_type::pointer_type;

    if (node_type::max_label < label) {
      throw invalid_argument("Cannot represent that large a label");
    }

    shared_levelized_file<node_type> nf;
    {
      node_writer nw(nf);
      nw.unsafe_push(node(label, pointer_type::max_id, pointer_type(false), pointer_type(true)));
      nw.unsafe_push(level_info(label,1u));
      nw.unsafe_set_canonical(true);
    }

    return nf;
  }

  template<typename dd_policy, bool INIT_TERMINAL = false, bool HIGH_VAL = true>
  class chain_low : public dd_policy
  {
  public:
    static constexpr bool init_terminal = INIT_TERMINAL;

    constexpr bool
    skip(const typename dd_policy::label_type &) const
    { return false; }

    inline typename dd_policy::node_type
    make_node(const typename dd_policy::label_type &l,
              const typename dd_policy::pointer_type &r) const
    {
      return typename dd_policy::node_type(l, dd_policy::max_id,
                                        r,
                                        typename dd_policy::pointer_type(HIGH_VAL));
    }
  };

  template<typename dd_policy, bool INIT_TERMINAL = true, bool LOW_VAL = false>
  class chain_high : public dd_policy
  {
  public:
    static constexpr bool init_terminal = INIT_TERMINAL;

    constexpr bool
    skip(const typename dd_policy::label_type &) const
    { return false; }

    inline typename dd_policy::node_type
    make_node(const typename dd_policy::label_type &l,
              const typename dd_policy::pointer_type &r) const
    {
      return typename dd_policy::node_type(l, dd_policy::max_id,
                                        typename dd_policy::pointer_type(LOW_VAL),
                                        r);
    }
  };

  template<typename dd_policy, bool INIT_TERMINAL = true>
  class chain_both : public dd_policy
  {
  public:
    static constexpr bool init_terminal = INIT_TERMINAL;

    constexpr bool
    skip(const typename dd_policy::label_type &) const
    { return false; }

    inline typename dd_policy::node_type
    make_node(const typename dd_policy::label_type &l,
              const typename dd_policy::pointer_type &r) const
    {
      return typename dd_policy::node_type(l, dd_policy::max_id, r, r);
    }
  };

  template<typename chain_policy>
  inline typename chain_policy::reduced_t
  build_chain(const chain_policy &policy,
              const generator<typename chain_policy::reduced_t::label_type> &vars)
  {
    typename chain_policy::label_type next_label = vars();

    if (chain_policy::max_label < next_label) {
      return build_terminal<chain_policy>(chain_policy::init_terminal);
    }

    shared_levelized_file<typename chain_policy::node_type> nf;
    node_writer nw(nf);

    size_t max_internal_cut      = 1;

    size_t terminals_internal[2] = {0u, 0u};
    bool   terminal_at_bottom[2] = {false, false};
    size_t terminals[2]          = {0u, 0u};

    node::pointer_type root = node::pointer_type(chain_policy::init_terminal);

    while(next_label <= chain_policy::max_label) {
      // Fail if generator is increasing.
      if (!root.is_terminal() && root.label() < next_label) {
        throw invalid_argument("Labels not given in decreasing order");
      }

      // Skip value if generator provides the same (legal) value twice.
      if (!root.is_terminal() && root.label() == next_label) {
        next_label = vars();
        continue;
      }

      // Skip value, if policy calls for it.
      if (policy.skip(next_label)) {
        next_label = vars();
        continue;
      }

      // Create node on chain.
      const node n = policy.make_node(next_label, root);

      adiar_assert(n.label() == next_label, "Policy ought to make a node for this level node");
      adiar_assert(n.id() == node::max_id,  "Policy ought to make a canonical node");

      max_internal_cut = std::max<size_t>(max_internal_cut,
                                          n.low().is_node() + n.high().is_node());

      if (n.low().is_terminal()) {
        terminals_internal[n.low().value()] =
          std::max<size_t>(terminals_internal[n.low().value()], n.high().is_node());

        terminals[n.low().value()] += 1u;
      }

      if (n.high().is_terminal()) {
        terminals_internal[n.high().value()] =
          std::max<size_t>(terminals_internal[n.high().value()], n.low().is_node());

        terminals[n.high().value()] += 1u;
      }

      if (root.is_terminal()) {
        terminal_at_bottom[n.low().value()] = true;
        terminal_at_bottom[n.high().value()] = true;
      }

      nw.unsafe_push(n);
      nw.unsafe_push(level_info(next_label, 1u));

      root = n.uid();

      // Get next label
      next_label = vars();
    }

    // If all values have been skipped by the policy, then collapse to a terminal
    if (nw.size() == 0u) {
      return build_terminal<chain_policy>(chain_policy::init_terminal);
    }

    // Canonicity (assuming the policies are correct)
    nw.unsafe_set_canonical(true);

    // 1-level cuts
    nf->max_1level_cut[cut::Internal] = max_internal_cut;

    nf->max_1level_cut[cut::Internal_False] =
      std::max<size_t>({
          nf->max_1level_cut[cut::Internal],
          terminals_internal[false] + terminals[false] - terminal_at_bottom[false],
          terminals[false]
        });

    nf->max_1level_cut[cut::Internal_True] =
      std::max<size_t>({
          nf->max_1level_cut[cut::Internal],
          terminals_internal[true] + terminals[true] - terminal_at_bottom[true],
          terminals[true]
        });

    nf->max_1level_cut[cut::All] =
      std::max<size_t>({
          nf->max_1level_cut[cut::Internal],
          nf->max_1level_cut[cut::Internal_False],
          nf->max_1level_cut[cut::Internal_True],
          terminals[false] + terminals[true]
        });

    return nf;
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_BUILD_H

#ifndef ADIAR_INTERNAL_ALGORITHMS_BUILD_H
#define ADIAR_INTERNAL_ALGORITHMS_BUILD_H

#include <adiar/exception.h>
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
  shared_levelized_file<typename dd_policy::node_t>
  build_terminal(bool value)
  {
    using node_t = typename dd_policy::node_t;
    shared_levelized_file<node_t> nf;
    {
      node_writer nw(nf);
      nw.unsafe_push(node_t(value));
      nw.unsafe_set_number_of_terminals(!value, value);
      nw.unsafe_set_canonical(true);
    }

    return nf;
  }

  template<typename dd_policy>
  inline
  shared_levelized_file<typename dd_policy::node_t>
  build_ithvar(typename dd_policy::label_t label)
  {
    using node_t = typename dd_policy::node_t;
    using ptr_t = typename node_t::ptr_t;

    if (node_t::MAX_LABEL < label) {
      throw invalid_argument("Cannot represent that large a label");
    }

    shared_levelized_file<node_t> nf;
    {
      node_writer nw(nf);
      nw.unsafe_push(node(label, ptr_t::MAX_ID, ptr_t(false), ptr_t(true)));
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
    skip(const typename dd_policy::label_t &) const
    { return false; }

    inline typename dd_policy::node_t
    make_node(const typename dd_policy::label_t &l,
              const typename dd_policy::ptr_t &r) const
    {
      return typename dd_policy::node_t(l, dd_policy::MAX_ID,
                                        r,
                                        typename dd_policy::ptr_t(HIGH_VAL));
    }
  };

  template<typename dd_policy, bool INIT_TERMINAL = true, bool LOW_VAL = false>
  class chain_high : public dd_policy
  {
  public:
    static constexpr bool init_terminal = INIT_TERMINAL;

    constexpr bool
    skip(const typename dd_policy::label_t &) const
    { return false; }

    inline typename dd_policy::node_t
    make_node(const typename dd_policy::label_t &l,
              const typename dd_policy::ptr_t &r) const
    {
      return typename dd_policy::node_t(l, dd_policy::MAX_ID,
                                        typename dd_policy::ptr_t(LOW_VAL),
                                        r);
    }
  };

  template<typename dd_policy, bool INIT_TERMINAL = true>
  class chain_both : public dd_policy
  {
  public:
    static constexpr bool init_terminal = INIT_TERMINAL;

    constexpr bool
    skip(const typename dd_policy::label_t &) const
    { return false; }

    inline typename dd_policy::node_t
    make_node(const typename dd_policy::label_t &l,
              const typename dd_policy::ptr_t &r) const
    {
      return typename dd_policy::node_t(l, dd_policy::MAX_ID, r, r);
    }
  };

  template<typename chain_policy>
  inline shared_levelized_file<typename chain_policy::node_t>
  build_chain(const chain_policy &policy,
              const shared_file<node::label_t> &labels)
    {
      const size_t number_of_levels = labels->size();
      if (number_of_levels == 0) {
        return build_terminal<chain_policy>(chain_policy::init_terminal);
      }

      shared_levelized_file<typename chain_policy::node_t> nf;
      node_writer nw(nf);

      file_stream<node::label_t, true> ls(labels);

      size_t max_internal_cut      = 1;

      size_t terminals_internal[2] = {0u, 0u};
      bool   terminal_at_bottom[2] = {false, false};
      size_t terminals[2]          = {0u, 0u};

      node::ptr_t root = node::ptr_t(chain_policy::init_terminal);

      while(ls.can_pull()) {
        const node::ptr_t::label_t next_label = ls.pull();

        if (node::MAX_LABEL < next_label) {
          throw invalid_argument("Cannot represent that large a label");
        }
        if (!root.is_terminal() && root.label() <= next_label) {
          throw invalid_argument("Labels not given in increasing order");
        }

        if (policy.skip(next_label)) { continue; }

        const node n = policy.make_node(next_label, root);

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
        nw.unsafe_push(level_info(next_label,1u));

        root = n.uid();
      }

      if (nw.size() == 0u) {
        return build_terminal<chain_policy>(chain_policy::init_terminal);
      }

      nw.unsafe_set_canonical(true);

      // 1-level cuts
      nf->max_1level_cut[cut_type::INTERNAL] = max_internal_cut;

      nf->max_1level_cut[cut_type::INTERNAL_FALSE] =
        std::max<size_t>({
            nf->max_1level_cut[cut_type::INTERNAL],
            terminals_internal[false] + terminals[false] - terminal_at_bottom[false],
            terminals[false]
          });

      nf->max_1level_cut[cut_type::INTERNAL_TRUE] =
        std::max<size_t>({
            nf->max_1level_cut[cut_type::INTERNAL],
            terminals_internal[true] + terminals[true] - terminal_at_bottom[true],
            terminals[true]
          });

      nf->max_1level_cut[cut_type::ALL] =
        std::max<size_t>({
            nf->max_1level_cut[cut_type::INTERNAL],
            nf->max_1level_cut[cut_type::INTERNAL_FALSE],
            nf->max_1level_cut[cut_type::INTERNAL_TRUE],
            terminals[false] + terminals[true]
          });

      return nf;
    }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_BUILD_H

#ifndef ADIAR_INTERNAL_ALGORITHMS_BUILD_H
#define ADIAR_INTERNAL_ALGORITHMS_BUILD_H

#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/node_writer.h>
#include <adiar/internal/io/shared_file_ptr.h>

namespace adiar::internal
{
  inline shared_levelized_file<node> build_terminal(bool value)
  {
    shared_levelized_file<node> nf;
    node_writer nw(nf);
    nw.unsafe_push(node(value));

    nf->number_of_terminals[value] = 1;

    return nf;
  }

  inline shared_levelized_file<node> build_ithvar(ptr_uint64::label_t label)
  {
    adiar_assert(label <= node::ptr_t::MAX_LABEL, "Cannot represent that large a label");

    shared_levelized_file<node> nf;
    {
      node_writer nw(nf);
      nw.unsafe_push(node(label, node::ptr_t::MAX_ID,
                          ptr_uint64(false),
                          ptr_uint64(true)));

      nw.unsafe_push(level_info(label,1u));
    }

    return nf;
  }

  template<typename dd_policy, bool INIT_TERMINAL = false, bool HIGH_VAL = true>
  class chain_low : public dd_policy
  {
  public:
    static constexpr bool init_terminal = INIT_TERMINAL;

    inline
    typename dd_policy::node_t
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

    inline
    typename dd_policy::node_t
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

    inline
    typename dd_policy::node_t
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
        return build_terminal(chain_policy::init_terminal);
      }

      shared_levelized_file<typename chain_policy::node_t> nf;
      node_writer nw(nf);

      file_stream<node::label_t, true> ls(labels);

      size_t max_internal_cut    = 1;
      size_t terminals[2] = {0u, 0u};

      node::ptr_t root = node::ptr_t(chain_policy::init_terminal);
      while(ls.can_pull()) {
        const node::ptr_t::label_t next_label = ls.pull();

        adiar_assert(root.is_terminal() || next_label < root.label(),
                     "Labels not given in increasing order");

        const node n = policy.make_node(next_label, root);

        max_internal_cut = std::max<size_t>(max_internal_cut,
                                            n.low().is_node() + n.high().is_node());

        if (n.low().is_terminal()) {
          terminals[n.low().value()] += 1u;
        }
        if (n.high().is_terminal()) {
          terminals[n.high().value()] += 1u;
        }

        nw.unsafe_push(n);
        nw.unsafe_push(level_info(next_label,1u));

        root = n.uid();
      }

      // 1-level cuts
      nf->max_1level_cut[cut_type::INTERNAL] = max_internal_cut;

      nf->max_1level_cut[cut_type::INTERNAL_FALSE] = std::max(max_internal_cut,
                                                              terminals[false]);

      nf->max_1level_cut[cut_type::INTERNAL_TRUE] = std::max(max_internal_cut,
                                                             terminals[true]);

      nf->max_1level_cut[cut_type::ALL] = std::max(max_internal_cut,
                                                   terminals[false] + terminals[true]);

      return nf;
    }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_BUILD_H

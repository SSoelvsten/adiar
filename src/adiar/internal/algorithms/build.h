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

  template<typename chain_policy>
  inline shared_levelized_file<node> build_chain(const chain_policy &/*policy*/,
                                                 const shared_file<node::label_t> &labels)
    {
      const size_t number_of_levels = labels->size();
      if (number_of_levels == 0) {
        return build_terminal(chain_policy::on_empty);
      }

      ptr_uint64 low = ptr_uint64(chain_policy::terminal_value[false]);
      ptr_uint64 high = ptr_uint64(chain_policy::terminal_value[true]);

      shared_levelized_file<bdd::node_t> nf;
      node_writer nw(nf);

      file_stream<node::label_t, true> ls(labels);
      while(ls.can_pull()) {
        node::ptr_t::label_t next_label = ls.pull();
        node next_node = node(next_label, node::ptr_t::MAX_ID, low, high);

        adiar_assert(next_label <= node::ptr_t::MAX_LABEL, "Cannot represent that large a label");
        adiar_assert(high.is_terminal() || next_label < high.label(),
                     "Labels not given in increasing order");

        if constexpr (chain_policy::link[false]) {
          low = next_node.uid();
        }
        if constexpr (chain_policy::link[true]) {
          high = next_node.uid();
        }

        nw.unsafe_push(next_node);
        nw.unsafe_push(level_info(next_label,1u));
      }

      // Compute 1-level cut sizes better than 'nw.detach()' will do on return.
      const size_t internal_arcs =
        number_of_levels > 1 ? (chain_policy::link[false] + chain_policy::link[true]) : 1u;

      nf->max_1level_cut[cut_type::INTERNAL] = internal_arcs;

      const size_t false_arcs_pre_end =
        (number_of_levels - 1) * ((!chain_policy::link[false] && !chain_policy::terminal_value[false])
                                  + (!chain_policy::link[true] && !chain_policy::terminal_value[true]));

      const size_t false_arcs_end =
        false_arcs_pre_end + !chain_policy::terminal_value[false] + !chain_policy::terminal_value[true];

      nf->max_1level_cut[cut_type::INTERNAL_FALSE] = std::max(internal_arcs + false_arcs_pre_end,
                                                              false_arcs_end);

      const size_t true_arcs_pre_end  =
        (number_of_levels - 1) * ((!chain_policy::link[false] && chain_policy::terminal_value[false])
                                  + (!chain_policy::link[true] && chain_policy::terminal_value[true]));

      const size_t true_arcs_end =
        true_arcs_pre_end + chain_policy::terminal_value[false] + chain_policy::terminal_value[true];

      nf->max_1level_cut[cut_type::INTERNAL_TRUE] = std::max(internal_arcs + true_arcs_pre_end,
                                                             true_arcs_end);

      nf->max_1level_cut[cut_type::ALL] = std::max(internal_arcs + false_arcs_pre_end + true_arcs_pre_end,
                                                   false_arcs_end + true_arcs_end);

      return nf;
    }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_BUILD_H

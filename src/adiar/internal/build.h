#ifndef ADIAR_INTERNAL_BUILD_H
#define ADIAR_INTERNAL_BUILD_H

#include <adiar/data.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>

namespace adiar
{
  inline node_file build_sink(bool value)
  {
    node_file nf;
    node_writer nw(nf);
    nw.unsafe_push(create_sink(value));

    nf->number_of_sinks[value] = 1;

    return nf;
  }

  inline node_file build_ithvar(label_t label)
  {
    adiar_assert(label <= MAX_LABEL, "Cannot represent that large a label");

    node_file nf;
    node_writer nw(nf);
    nw.unsafe_push(create_node(label, MAX_ID,
                               create_sink_ptr(false),
                               create_sink_ptr(true)));

    nw.unsafe_push(create_level_info(label,1u));

    return nf;
  }

  template<bool on_empty, bool link_low, bool link_high,
    bool low_sink_value = false,
    bool high_sink_value = true>
  inline node_file build_chain(const label_file &labels)
    {
      const size_t number_of_levels = labels.size();
      if (number_of_levels == 0) {
        return build_sink(on_empty);
      }

      ptr_t low = create_sink_ptr(low_sink_value);
      ptr_t high = create_sink_ptr(high_sink_value);

      node_file nf;
      node_writer nw(nf);

      label_stream<true> ls(labels);
      while(ls.can_pull()) {
        label_t next_label = ls.pull();
        node_t next_node = create_node(next_label, MAX_ID, low, high);

        adiar_assert(next_label <= MAX_LABEL, "Cannot represent that large a label");
        adiar_assert(is_sink(high) || next_label < label_of(high),
                     "Labels not given in increasing order");

        if constexpr(link_low) {
          low = next_node.uid;
        }
        if constexpr(link_high) {
          high = next_node.uid;
        }

        nw.unsafe_push(next_node);
        nw.unsafe_push(create_level_info(next_label,1u));
      }

      // Compute 1-level cut sizes better than 'nw.detach()' will do on return.
      const size_t internal_arcs = number_of_levels > 1 ? (link_low + link_high) : 1u;

      nf->max_1level_cut[cut_type::INTERNAL] = internal_arcs;

      const size_t false_arcs_pre_end =
        (number_of_levels - 1) * ((!link_low && !low_sink_value) + (!link_high && !high_sink_value));

      const size_t false_arcs_end = false_arcs_pre_end + !low_sink_value + !high_sink_value;

      nf->max_1level_cut[cut_type::INTERNAL_FALSE] = std::max(internal_arcs + false_arcs_pre_end,
                                                              false_arcs_end);

      const size_t true_arcs_pre_end  =
      (number_of_levels - 1) * ((!link_low && low_sink_value) + (!link_high && high_sink_value));

      const size_t true_arcs_end = true_arcs_pre_end + low_sink_value + high_sink_value;

      nf->max_1level_cut[cut_type::INTERNAL_TRUE] = std::max(internal_arcs + true_arcs_pre_end,
                                                             true_arcs_end);

      nf->max_1level_cut[cut_type::ALL] = std::max(internal_arcs + false_arcs_pre_end + true_arcs_pre_end,
                                                   false_arcs_end + true_arcs_end);

      return nf;
    }
}

#endif // ADIAR_INTERNAL_BUILD_H

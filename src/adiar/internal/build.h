#ifndef ADIAR_INTERNAL_BUILD_H
#define ADIAR_INTERNAL_BUILD_H

#include <adiar/data.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/assert.h>

namespace adiar
{
  inline node_file build_sink(bool value)
  {
    node_file nf;
    node_writer nw(nf);
    nw.unsafe_push(create_sink(value));
    return nf;
  }

  inline node_file build_ithvar(label_t label)
  {
    adiar_assert(label <= MAX_LABEL, "Cannot represent that large a label");

    node_file nf;
    node_writer nw(nf);
    nw.unsafe_push(create_node(label, 0, create_sink_ptr(false), create_sink_ptr(true)));
    nw.unsafe_push(create_level_info(label,1u));
    return nf;
  }

  template<bool on_empty, bool link_low, bool link_high,
    bool init_low = false,
    bool init_high = true>
  inline node_file build_chain(const label_file &labels)
    {
      if (labels.size() == 0) {
        return build_sink(on_empty);
      }

      ptr_t low = create_sink_ptr(init_low);
      ptr_t high = create_sink_ptr(init_high);

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

      return nf;
    }

}

#endif // ADIAR_INTERNAL_BUILD_H

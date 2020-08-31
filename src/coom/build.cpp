#ifndef COOM_MAKE_H
#define COOM_MAKE_H

#include <tpie/file_stream.h>

#include "data.h"
#include "assert.h"

#include "build.h"

namespace coom {
  namespace assert {
    void is_build_counter_valid_labels (label_t min_label, label_t max_label, uint64_t threshold) {
      assert(min_label <= max_label);
      assert(threshold <= max_label - min_label + 1);
    }
  }

  void build_x(label_t label, tpie::file_stream<node_t> &out_nodes)
  {
    assert::is_valid_output_stream(out_nodes);
    out_nodes.write(create_node(label, 0, create_sink_ptr(false), create_sink_ptr(true)));
  }

  void build_not_x(label_t label, tpie::file_stream<node_t> &out_nodes)
  {
    assert::is_valid_output_stream(out_nodes);
    out_nodes.write(create_node(label, 0, create_sink_ptr(true), create_sink_ptr(false)));
  }

  void build_and(tpie::file_stream<label_t> &in_labels,
                 tpie::file_stream<node_t> &out_nodes)
  {
    assert::is_valid_output_stream(out_nodes);
    if (in_labels.size() == 0) {
      out_nodes.write(create_sink(true));
      return;
    }

    ptr_t low = create_sink_ptr(false);
    ptr_t high = create_sink_ptr(true);

    in_labels.seek(0, tpie::file_stream_base::end);

    while(in_labels.can_read_back()) {
      node_t next_node = create_node(in_labels.read_back(), 0, low, high);
      high = next_node.uid;
      out_nodes.write(next_node);
    }
  }

  void build_or(tpie::file_stream<label_t> &in_labels,
                 tpie::file_stream<node_t> &out_nodes)
  {
    assert::is_valid_output_stream(out_nodes);
    if (in_labels.size() == 0) {
      out_nodes.write(create_sink(false));
      return;
    }

    ptr_t low = create_sink_ptr(false);
    ptr_t high = create_sink_ptr(true);

    in_labels.seek(0, tpie::file_stream_base::end);

    while(in_labels.can_read_back()) {
      node_t next_node = create_node(in_labels.read_back(), 0, low, high);
      low = next_node.uid;
      out_nodes.write(next_node);
    }
  }

  inline id_t build_counter_min_id(label_t label, label_t max_label, uint64_t threshold)
  {
    return label > max_label - threshold
      ? threshold - (max_label - label + 1)
      : 0;
  }

  void build_counter(label_t min_label, label_t max_label,
                     uint64_t threshold,
                     tpie::file_stream<node_t> &out_nodes)
  {
    assert::is_valid_output_stream(out_nodes);
    assert::is_build_counter_valid_labels(min_label, max_label, threshold);

    label_t curr_label = max_label;

    ptr_t gt_sink = create_sink_ptr(false); // create_sink(comparator(threshold + 1, threshold));
    ptr_t eq_sink = create_sink_ptr(true);  // create_sink(comparator(threshold, threshold));
    ptr_t lt_sink = create_sink_ptr(false); // create_sink(comparator(threshold - 1, threshold));

    do {
      // Start with the maximal number the accumulated value can be at
      // up to this label.
      id_t curr_id = std::min(curr_label, threshold);

      // How small has the accumulated sum up to this point to be, such
      // that it is still possible to reach threshold before max_label?
      id_t min_id = build_counter_min_id(curr_label, max_label, threshold);

      do {
        ptr_t low;
        if (curr_label == max_label) {
          low = curr_id == threshold ? eq_sink : lt_sink;
        } else if (curr_id < build_counter_min_id(curr_label+1, max_label, threshold)) {
          low = lt_sink;
        } else {
          low = coom::create_node_ptr(curr_label + 1, curr_id);
        }

        ptr_t high;
        if (curr_label == max_label) {
          high = curr_id + 1 == threshold ? eq_sink : gt_sink;
        } else if (curr_id == threshold) {
          high = gt_sink;
        } else {
          high = coom::create_node_ptr(curr_label + 1, curr_id + 1);
        }

        out_nodes.write(coom::create_node(curr_label, curr_id, low, high));

      } while (curr_id-- > min_id);
    } while (curr_label-- > min_label);
  }
}

#endif // COOM_BUILD_H

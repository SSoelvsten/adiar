#ifndef COOM_BUILD_CPP
#define COOM_BUILD_CPP

#include "build.h"

#include <coom/file_stream.h>
#include <coom/file_writer.h>

#include <coom/bdd/negate.h>

#include <coom/assert.h>

namespace coom {
  // TODO: Memoization table for the sink, ithvar, and nithvar builders

  bdd bdd_sink(bool value)
  {
    node_file nf;
    node_writer nw(nf);
    nw.unsafe_push(create_sink(value));
    return nf;
  }

  bdd bdd_true()
  {
    return bdd_sink(true);
  }

  bdd bdd_false()
  {
    return bdd_sink(false);
  }

  bdd bdd_ithvar(label_t label)
  {
    node_file nf;
    node_writer nw(nf);
    nw.unsafe_push(create_node(label, 0, create_sink_ptr(false), create_sink_ptr(true)));
    nw.unsafe_push(meta_t {label});
    return nf;
  }

  bdd bdd_nithvar(label_t label)
  {
    return bdd_not(bdd_ithvar(label));
  }

  bdd bdd_and(const label_file &labels)
  {
    if (labels.size() == 0) {
      return bdd_sink(true);
    }

    ptr_t low = create_sink_ptr(false);
    ptr_t high = create_sink_ptr(true);

    node_file nf;
    node_writer nw(nf);

    label_stream<true> ls(labels);
    while(ls.can_pull()) {
      label_t next_label = ls.pull();
      node_t next_node = create_node(next_label, 0, low, high);

      high = next_node.uid;

      nw.unsafe_push(next_node);
      nw.unsafe_push(meta_t {next_label});
    }

    return nf;
  }

  bdd bdd_or(const label_file &labels)
  {
    if (labels.size() == 0) {
      return bdd_sink(false);
    }

    ptr_t low = create_sink_ptr(false);
    ptr_t high = create_sink_ptr(true);

    node_file nf;
    node_writer nw(nf);

    label_stream<true> ls(labels);
    while(ls.can_pull()) {
      label_t next_label = ls.pull();
      node_t next_node = create_node(next_label, 0, low, high);

      low = next_node.uid;

      nw.unsafe_push(next_node);
      nw.unsafe_push(meta_t {next_label});
    }

    return nf;
  }

  inline id_t bdd_counter_min_id(label_t label, label_t max_label, uint64_t threshold)
  {
    return label > max_label - threshold
      ? threshold - (max_label - label + 1)
      : 0;
  }

  bdd bdd_counter(label_t min_label, label_t max_label, uint64_t threshold)
  {
#if COOM_ASSERT
    assert(threshold < MAX_ID);
    assert(min_label <= max_label);
    assert(threshold <= max_label - min_label + 1);
#endif
    node_file nf;
    node_writer nw(nf);

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
      id_t min_id = bdd_counter_min_id(curr_label, max_label, threshold);

      do {
        ptr_t low;
        if (curr_label == max_label) {
          low = curr_id == threshold ? eq_sink : lt_sink;
        } else if (curr_id < bdd_counter_min_id(curr_label+1, max_label, threshold)) {
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

        nw.unsafe_push(coom::create_node(curr_label, curr_id, low, high));

      } while (curr_id-- > min_id);
      nw.unsafe_push(meta_t {curr_label});
    } while (curr_label-- > min_label);

    return nf;
  }
}

#endif // COOM_BUILD_CPP

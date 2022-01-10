#include <adiar/bdd.h>

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/build.h>

#include <adiar/assert.h>

namespace adiar
{
  bdd bdd_sink(bool value)
  {
    return build_sink(value);
  }

  bdd bdd_true()
  {
    return build_sink(true);
  }

  bdd bdd_false()
  {
    return build_sink(false);
  }

  bdd bdd_ithvar(label_t label)
  {
    return build_ithvar(label);
  }

  bdd bdd_nithvar(label_t label)
  {
    return bdd_not(build_ithvar(label));
  }

  bdd bdd_and(const label_file &labels)
  {
    return build_chain<true, false, true>(labels);
  }

  bdd bdd_or(const label_file &labels)
  {
    return build_chain<false, true, false>(labels);
  }

  inline id_t bdd_counter_min_id(label_t label, label_t max_label, uint64_t threshold)
  {
    return label > max_label - threshold
      ? threshold - (max_label - label + 1)
      : 0;
  }

  bdd bdd_counter(label_t min_label, label_t max_label, label_t threshold)
  {
    adiar_assert(min_label <= max_label,
                 "The given min_label should be smaller than the given max_label");

    ptr_t gt_sink = create_sink_ptr(false); // create_sink(comparator(threshold + 1, threshold));
    ptr_t eq_sink = create_sink_ptr(true);  // create_sink(comparator(threshold, threshold));
    ptr_t lt_sink = create_sink_ptr(false); // create_sink(comparator(threshold - 1, threshold));

    if (max_label - min_label + 1 < threshold) {
      return bdd_sink(false);
    }

    node_file nf;
    node_writer nw(nf);

    label_t curr_label = max_label;

    do {
      // Start with the maximal number the accumulated value can be at
      // up to this label.
      id_t max_id = std::min(curr_label - min_label, threshold);
      id_t curr_id = max_id;

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
          low = adiar::create_node_ptr(curr_label + 1, curr_id);
        }

        ptr_t high;
        if (curr_label == max_label) {
          high = curr_id + 1 == threshold ? eq_sink : gt_sink;
        } else if (curr_id == threshold) {
          high = gt_sink;
        } else {
          high = adiar::create_node_ptr(curr_label + 1, curr_id + 1);
        }

        nw.unsafe_push(adiar::create_node(curr_label, curr_id, low, high));

      } while (curr_id-- > min_id);
      nw.unsafe_push(create_level_info(curr_label, (max_id - min_id) + 1));
    } while (curr_label-- > min_label);

    return nf;
  }
}

#include <adiar/bdd.h>

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/build.h>
#include <adiar/internal/cut.h>

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

    const ptr_t gt_sink = create_sink_ptr(false);
    const ptr_t eq_sink = create_sink_ptr(true);
    const ptr_t lt_sink = create_sink_ptr(false);

    const size_t vars = max_label - min_label + 1u;
    if (vars < threshold) {
      return bdd_sink(false);
    }

    if (vars == 1) {
      adiar_debug(min_label == max_label,
                  "If 'vars == 1' then we ought to have 'max_label - min_label == 0'");
      return threshold == 1 ? bdd_ithvar(min_label) : bdd_nithvar(min_label);
    }

    // Construct parallelogram-shaped BDD where each node stores the number of
    // variables up to said point has been set to true.
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

    // Maximum 1-level cut
    const label_t first_lvl_with_lt = vars - threshold; // 0-indexed
    const label_t first_lvl_with_gt = threshold;        // 0-indexed

    // A single gt_sink is created on each level after having seen threshold+1
    // many levels (including said level).
    const size_t gt_sinks = vars - first_lvl_with_gt;

    // There are two nodes (only one if the threshold is 0 or vars) at the very
    // bottom that can reach the eq_sink.
    const size_t eq_sinks = 2u - (threshold == 0u || threshold == vars);

    // An lt_sink is made once on each level for the node that is i levels from
    // the end but still needs threshold-i+1 many variable to be set to true.
    const size_t lt_sinks = threshold;

    const label_t shallowest_widest_lvl = std::min(first_lvl_with_lt, first_lvl_with_gt);

    const size_t internal_cut_below_shallowest_lvl = 2u * (shallowest_widest_lvl + 1u)
      // Do not count the one gt_sink (if any)
      - (first_lvl_with_gt == shallowest_widest_lvl)
      // Do not count the eq_sinks (if any)
      - (vars == shallowest_widest_lvl + 1u ? eq_sinks : 0u)
      // Do not count the lt_sink (if any)
      - (first_lvl_with_lt == shallowest_widest_lvl);

    const label_t deepest_widest_lvl = std::max(first_lvl_with_lt, first_lvl_with_gt);

    const size_t internal_cut_below_deepest_lvl = 2u * (threshold + 1u)
      // Do not count nodes that do not exist due to shortcutting to lt_sink.
      - 2u * (deepest_widest_lvl - first_lvl_with_lt)
      // Do not count the one gt_sink (if any)
      - (gt_sinks > 0)
      // Do not count the eq_sinks (if any)
      - (vars <= deepest_widest_lvl+1u ? eq_sinks : 0u)
      // Do not count the one lt_sink (if any)
      - (lt_sinks > 0);

    nf->max_1level_cut[cut_type::INTERNAL] = std::max(internal_cut_below_shallowest_lvl,
                                                      internal_cut_below_deepest_lvl);

    // With 'vars - deepest_widest_lvl' we obtain the number of levels beyond
    // the widest one. But, if 'deepest_widest_lvl < vars' then there are two
    // nodes at the last level which offsets the number of levels by one more.
    const label_t lvls_after_widest = vars - deepest_widest_lvl - (deepest_widest_lvl < vars);

    // The maximum cut with false sinks is at the deepes widest level. Beyond
    // it, a node (with two children) is removed, which outweighs the gt_sink
    // and possible lt_sink added.
    nf->max_1level_cut[cut_type::INTERNAL_FALSE] =
      std::max(internal_cut_below_deepest_lvl + lt_sinks + gt_sinks - 2u * lvls_after_widest,
               lt_sinks + gt_sinks);

    // Compare the cut at deepest widest level and below the last level.
    nf->max_1level_cut[cut_type::INTERNAL_TRUE] = std::max(nf->max_1level_cut[cut_type::INTERNAL],
                                                           eq_sinks);

    // Counting both false and true sinks is only different from counting false
    // sinks, if the number of eq_sinks outweighs the number of internal nodes
    // since the true sinks are only spawned at the very bottom.
    nf->max_1level_cut[cut_type::ALL] = std::max(nf->max_1level_cut[cut_type::INTERNAL_FALSE],
                                                 lt_sinks + eq_sinks + gt_sinks);

    return nf;
  }
}

#include <adiar/bdd.h>

#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/levelized_file_writer.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/cut.h>
#include <adiar/internal/algorithms/build.h>
#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/data_types/ptr.h>

namespace adiar
{
  bdd bdd_terminal(bool value)
  {
    return internal::build_terminal(value);
  }

  bdd bdd_true()
  {
    return internal::build_terminal(true);
  }

  bdd bdd_false()
  {
    return internal::build_terminal(false);
  }

  bdd bdd_ithvar(bdd::label_t label)
  {
    return internal::build_ithvar(label);
  }

  bdd bdd_nithvar(bdd::label_t label)
  {
    return bdd_not(internal::build_ithvar(label));
  }

  bdd bdd_and(const shared_file<bdd::label_t> &labels)
  {
    return internal::build_chain<true, false, true>(labels);
  }

  bdd bdd_or(const shared_file<bdd::label_t> &labels)
  {
    return internal::build_chain<false, true, false>(labels);
  }

  inline id_t bdd_counter_min_id(bdd::label_t label, bdd::label_t max_var, uint64_t threshold)
  {
    return label > max_var - threshold
      ? threshold - (max_var - label + 1)
      : 0;
  }

  bdd bdd_counter(bdd::label_t min_var, bdd::label_t max_var, bdd::label_t threshold)
  {
    adiar_assert(min_var <= max_var,
                 "The given min_var should be smaller than the given max_var");

    const bdd::ptr_t gt_terminal = bdd::ptr_t(false);
    const bdd::ptr_t eq_terminal = bdd::ptr_t(true);
    const bdd::ptr_t lt_terminal = bdd::ptr_t(false);

    const size_t vars = max_var - min_var + 1u;
    if (vars < threshold) {
      return bdd_terminal(false);
    }

    if (vars == 1) {
      adiar_debug(min_var == max_var,
                  "If 'vars == 1' then we ought to have 'max_var - min_var == 0'");
      return threshold == 1 ? bdd_ithvar(min_var) : bdd_nithvar(max_var);
    }

    // Construct parallelogram-shaped BDD where each node stores the number of
    // variables up to said point has been set to true.
    internal::shared_levelized_file<bdd::node_t> nf;
    internal::node_writer nw(nf);

    bdd::label_t curr_label = max_var;

    do {
      // Start with the maximal number the accumulated value can be at
      // up to this label.
      id_t max_id = std::min(curr_label - min_var, threshold);
      id_t curr_id = max_id;

      // How small has the accumulated sum up to this point to be, such
      // that it is still possible to reach threshold before max_var?
      id_t min_id = bdd_counter_min_id(curr_label, max_var, threshold);

      do {
        bdd::ptr_t low;
        if (curr_label == max_var) {
          low = curr_id == threshold ? eq_terminal : lt_terminal;
        } else if (curr_id < bdd_counter_min_id(curr_label+1, max_var, threshold)) {
          low = lt_terminal;
        } else {
          low = adiar::bdd::ptr_t(curr_label + 1, curr_id);
        }

        bdd::ptr_t high;
        if (curr_label == max_var) {
          high = curr_id + 1 == threshold ? eq_terminal : gt_terminal;
        } else if (curr_id == threshold) {
          high = gt_terminal;
        } else {
          high = bdd::ptr_t(curr_label + 1, curr_id + 1);
        }

        nw.unsafe_push(bdd::node_t(curr_label, curr_id, low, high));

      } while (curr_id-- > min_id);
      nw.unsafe_push(internal::level_info(curr_label, (max_id - min_id) + 1));

    } while (curr_label-- > min_var);

    // Maximum 1-level cut
    const bdd::label_t first_lvl_with_lt = vars - threshold; // 0-indexed
    const bdd::label_t first_lvl_with_gt = threshold;        // 0-indexed

    // A single gt_terminal is created on each level after having seen threshold+1
    // many levels (including said level).
    const size_t gt_terminals = vars - first_lvl_with_gt;

    // There are two nodes (only one if the threshold is 0 or vars) at the very
    // bottom that can reach the eq_terminal.
    const size_t eq_terminals = 2u - (threshold == 0u || threshold == vars);

    // An lt_terminal is made once on each level for the node that is i levels from
    // the end but still needs threshold-i+1 many variable to be set to true.
    const size_t lt_terminals = threshold;

    const bdd::label_t shallowest_widest_lvl = std::min(first_lvl_with_lt, first_lvl_with_gt);

    const size_t internal_cut_below_shallowest_lvl = 2u * (shallowest_widest_lvl + 1u)
      // Do not count the one gt_terminal (if any)
      - (first_lvl_with_gt == shallowest_widest_lvl)
      // Do not count the eq_terminals (if any)
      - (vars == shallowest_widest_lvl + 1u ? eq_terminals : 0u)
      // Do not count the lt_terminal (if any)
      - (first_lvl_with_lt == shallowest_widest_lvl);

    // The in-degree on the widest level is twice its width, except for the two
    // nodes on either end, since their in-degree is only 1 and not 2.
    const size_t internal_cut_above_shallowest_lvl = shallowest_widest_lvl == 0
      ? 1u
      : (2u * (shallowest_widest_lvl + 1u) - 2u);

    const bdd::label_t deepest_widest_lvl = std::max(first_lvl_with_lt, first_lvl_with_gt);

    const size_t internal_cut_below_deepest_lvl = 2u * (threshold + 1u)
      // Do not count nodes that do not exist due to shortcutting to lt_terminal.
      - 2u * (deepest_widest_lvl - first_lvl_with_lt)
      // Do not count the one gt_terminal (if any)
      - (gt_terminals > 0)
      // Do not count the eq_terminals (if any)
      - (vars <= deepest_widest_lvl+1u ? eq_terminals : 0u)
      // Do not count the one lt_terminal (if any)
      - (lt_terminals > 0);

    nf->max_1level_cut[internal::cut_type::INTERNAL] = std::max({
        internal_cut_above_shallowest_lvl,
        internal_cut_below_shallowest_lvl,
        internal_cut_below_deepest_lvl});

    // With 'vars - deepest_widest_lvl' we obtain the number of levels beyond
    // the widest one. But, if 'deepest_widest_lvl < vars' then there are two
    // nodes at the last level which offsets the number of levels by one more.
    const bdd::label_t lvls_after_widest = vars - deepest_widest_lvl - (deepest_widest_lvl < vars);

    // The maximum cut with false terminals is at the deepes widest level. Beyond
    // it, a node (with two children) is removed, which outweighs the gt_terminal
    // and possible lt_terminal added.
    nf->max_1level_cut[internal::cut_type::INTERNAL_FALSE] =
      std::max(internal_cut_below_deepest_lvl + lt_terminals + gt_terminals - 2u * lvls_after_widest,
               lt_terminals + gt_terminals);

    // Compare the cut at deepest widest level and below the last level.
    nf->max_1level_cut[internal::cut_type::INTERNAL_TRUE] = std::max(nf->max_1level_cut[internal::cut_type::INTERNAL],
                                                                     eq_terminals);

    // Counting both false and true terminals is only different from counting false
    // terminals, if the number of eq_terminals outweighs the number of internal nodes
    // since the true terminals are only spawned at the very bottom.
    nf->max_1level_cut[internal::cut_type::ALL] = std::max(nf->max_1level_cut[internal::cut_type::INTERNAL_FALSE],
                                                           lt_terminals + eq_terminals + gt_terminals);

    // Maximum 2-level cut
    //
    // Every node has at most in-degree of 2, which is also exactly it's
    // out-degree. The out-degree is also 2 (assuming we count all arcs). So, we
    // do not increase the size of the cut by abusing the added freedom of a
    // 2-level cut when all arcs are present.
    nf->max_2level_cut[internal::cut_type::ALL] = nf->max_1level_cut[internal::cut_type::ALL];

    // When only looking at internal arcs, then the exception to the above are
    // the nodes with id '0'and id 'threshold'. Both have only an out-degree of
    // 1, while the latter has an in-degree of one.
    //
    // This does not happen when in the following cases:
    const size_t extra_2level_cut =
      (// If we reach the widest level at the very bottom.
       vars > shallowest_widest_lvl + 1u
       // Except if each level only one node wide.
       && shallowest_widest_lvl > 0u
       ) ? 1u : 0u;

    nf->max_2level_cut[internal::cut_type::INTERNAL] = std::min(nf->max_1level_cut[internal::cut_type::ALL],
                                                                nf->max_1level_cut[internal::cut_type::INTERNAL] + extra_2level_cut);

    // When including the false terminal, then these two 'edge-case nodes' already
    // have an out-degree of two, except again if the same edge-case applies.
    nf->max_2level_cut[internal::cut_type::INTERNAL_FALSE] = std::min(nf->max_1level_cut[internal::cut_type::ALL],
                                                                      nf->max_1level_cut[internal::cut_type::INTERNAL_FALSE] + extra_2level_cut);

    // And similarly, if we only include the true terminals
    nf->max_2level_cut[internal::cut_type::INTERNAL_TRUE] = std::min(nf->max_1level_cut[internal::cut_type::ALL],
                                                                     nf->max_1level_cut[internal::cut_type::INTERNAL_TRUE] + extra_2level_cut);

    return nf;
  }
}

#ifndef ADIAR_ZDD_BUILD_H
#define ADIAR_ZDD_BUILD_H

#include <adiar/zdd.h>

#include <adiar/label.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/cut.h>

#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/data_types/node.h>

#include <functional>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a ZDD of all sets in the given domain who's size satisfy the
  /// given predicate.
  ///
  /// \param pred     The predicate whether to include a set of a certain size
  /// \param labels   The variable domain of interest
  /// \param set_size The threshold size to compare to
  //////////////////////////////////////////////////////////////////////////////
  template <typename pred_t = std::equal_to<label_t>>
  zdd zdd_sized_sets(const label_file &labels, label_t set_size,
                     pred_t pred = std::equal_to<label_t>())
  {
    // Generalisation of bdd_counter
    const size_t labels_size = labels.size();

    if (labels_size == 0) {
      return zdd_terminal(pred(0,set_size));
    }

    const bool lt_terminal_val = pred(set_size, set_size+1);
    const ptr_t lt_terminal = create_terminal_ptr(lt_terminal_val);

    if (labels_size < set_size) {
      return lt_terminal_val ? zdd_powerset(labels) : zdd_empty();
    }

    const bool eq_terminal_val = pred(set_size, set_size);
    const ptr_t eq_terminal = create_terminal_ptr(eq_terminal_val);

    if (labels_size == set_size) {
      if (lt_terminal_val == eq_terminal_val) {
        return eq_terminal_val ? zdd_powerset(labels) : zdd_empty();
      }
      if (eq_terminal_val) { return zdd_vars(labels); }
      // Notice, we don't return in the case of lt = T and eq = F.
    }

    const bool gt_terminal_val = pred(set_size + 1, set_size);
    const ptr_t gt_terminal = create_terminal_ptr(gt_terminal_val);

    if (lt_terminal_val && eq_terminal_val && gt_terminal_val) {
      return zdd_powerset(labels);
    }
    if (!lt_terminal_val && !eq_terminal_val && !gt_terminal_val) {
      return zdd_empty();
    }

    adiar_debug(lt_terminal_val || eq_terminal_val || gt_terminal_val,
                "Some set size must be allowed to exist");

    // Take care of the edge cases, where the construction below would collapse.
    if (set_size == 0) {
      if (!gt_terminal_val) { return zdd_terminal(eq_terminal_val); }
    }

    adiar_debug(set_size > 0 || gt_terminal_val,
                "Set size is only 0 if we accept a non-negative number of elements");

    if (set_size == 1) {
      if (!eq_terminal_val && !gt_terminal_val) { return zdd_null(); }
    }

    node_file nf;
    node_writer nw(nf);

    label_stream<true> ls(labels);

    // We use the id to store the number of elements, that we have already seen.
    // This does (often) make it non-canonical, but it does not seem worth the
    // effort and obscurity of code to fix it.
    //
    // Compute the maximal id (i.e. the maximal number of elements to count)
    // that should be accounted for in the ZDD. Anything past this should be
    // "reduced" away.
    const id_t max_id = set_size == 0                      ? gt_terminal_val   // gt_terminal_val == 1
                      : set_size == 1                      ? 2*gt_terminal_val
                      : gt_terminal_val && eq_terminal_val ? set_size
                      : gt_terminal_val                    ? set_size + 1u     // ~eq_terminal
                      : eq_terminal_val                    ? set_size - 1u     // ~gt_terminal
                                                           : set_size - 2u     // ~eq_terminal /\ ~gt_terminal
      ;

    const bool not_equal = lt_terminal_val && !eq_terminal_val && gt_terminal_val;

    id_t prior_min_id = MAX_ID; // <-- dummy value to squelch the compiler

    size_t processed_levels = 0u;
    label_t prior_label = MAX_LABEL; // <-- dummy value to squelch the compiler

    do {
      label_t curr_label = ls.pull();
      size_t level_size = 0u;

      // Start with the maximal number the accumulated value can be at
      // up to this level.
      const id_t remaining_levels = labels_size - processed_levels - 1; // exclusive of current level
      id_t curr_id = std::min(remaining_levels, max_id);

      // How small has the accumulated sum up to this point to be, such that it
      // is still possible to reach the last node before the last label?
      const id_t curr_level_width = processed_levels
        // Add node for reached-equality/-greater on high
        + ((eq_terminal_val && gt_terminal_val) || gt_terminal_val)
        // Add node for almost-never-reach-equality
        + (not_equal && processed_levels > 0)
        ;

      const id_t min_id = curr_level_width < max_id ? max_id - curr_level_width : 0u;

      do {
        ptr_t low;
        if (processed_levels == 0) { // lowest level
          low = curr_id == set_size+1 ? gt_terminal
              : curr_id == set_size   ? eq_terminal
              : lt_terminal;
        } else if (curr_id < prior_min_id) { // guaranteed to be in lt case
          if (not_equal) {
            low = curr_id == min_id
              ? create_node_ptr(prior_label, max_id)
              : lt_terminal; // <- When processed_levels == 1 and happens twice
          } else {
            low = lt_terminal_val
              ? create_node_ptr(prior_label, prior_min_id)
              : lt_terminal;
          }
        } else {
          low = create_node_ptr(prior_label, curr_id);
        }

        ptr_t high;
        if (processed_levels == 0) {
          high = curr_id >= set_size    ? gt_terminal
               : curr_id == set_size-1u ? eq_terminal
               : lt_terminal;
        } else if (curr_id == max_id) {
          high = gt_terminal_val ? low
               : eq_terminal_val ? eq_terminal
               : lt_terminal;
        } else if (not_equal && processed_levels == 1 && curr_id == min_id){
          high = lt_terminal; // <-- true terminal
        } else {
          high = create_node_ptr(prior_label, curr_id + 1u);
        }

        adiar_debug(high != create_terminal_ptr(false), "Should not create a reducible node");

        nw.unsafe_push(create_node(curr_label, curr_id, low, high));

        level_size++;
      } while (curr_id-- > min_id);

      adiar_debug(level_size > 0, "Should have output a node");
      nw.unsafe_push(create_level_info(curr_label, level_size));

      prior_label = curr_label;
      prior_min_id = curr_id + 1;

      processed_levels++;
    } while (ls.can_pull());

    // Maximum 1-level cut
    // TODO

    return nf;
  }
}

#endif // ADIAR_ZDD_BUILD_H

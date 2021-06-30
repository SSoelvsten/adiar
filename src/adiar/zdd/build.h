#ifndef ADIAR_ZDD_BUILD_H
#define ADIAR_ZDD_BUILD_H

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/zdd/zdd.h>

#include <functional>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a ZDD of only a single sink.
  ///
  /// \param value   The value for the sink. If true, then it represents {Ø} and
  ///                otherwise it will represent Ø.
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_sink(bool value);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create the ZDD that represents Ø.
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_empty();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create the ZDD that represents { Ø }.
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_null();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a ZDD for the family of the singleton set { i } only, i.e
  ///        a ZDD that represents { {i} }.
  ///
  /// \param label     The label of the variable x_i
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_ithvar(label_t label);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a ZDD for the family of the singleton set { i_1, i_2, ...,
  ///        i_k } only, i.e a ZDD that represents { {i_1, i_2, ..., i_k} }.
  ///
  /// \param label     The labels of the variables to include in this set
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_vars(const label_file &labels);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a ZDD for the family of all singleton sets { i_j }, i.e
  ///        a ZDD that represents { {x_i1}, {x_i2}, ..., {i_k} }.
  ///
  /// \param label     The label of the variable x_i
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_singletons(const label_file &labels);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create a ZDD for the power set of all variables.
  ///
  /// \param label     The label of the variable x_i
  //////////////////////////////////////////////////////////////////////////////
  zdd zdd_powerset(const label_file &labels);

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
      return zdd_sink(pred(0,set_size));
    }

    const bool lt_sink_val = pred(set_size, set_size+1);
    const ptr_t lt_sink = create_sink_ptr(lt_sink_val);

    if (labels_size < set_size) {
      return lt_sink_val ? zdd_powerset(labels) : zdd_empty();
    }

    const bool eq_sink_val = pred(set_size, set_size);
    const ptr_t eq_sink = create_sink_ptr(eq_sink_val);

    if (labels_size == set_size) {
      if (lt_sink_val == eq_sink_val) {
        return eq_sink_val ? zdd_powerset(labels) : zdd_empty();
      }
      if (eq_sink_val) { return zdd_vars(labels); }
      // Notice, we don't return in the case of lt = T and eq = F.
    }

    const bool gt_sink_val = pred(set_size + 1, set_size);
    const ptr_t gt_sink = create_sink_ptr(gt_sink_val);

    if (lt_sink_val && eq_sink_val && gt_sink_val) {
      return zdd_powerset(labels);
    }
    if (!lt_sink_val && !eq_sink_val && !gt_sink_val) {
      return zdd_empty();
    }

    adiar_debug(lt_sink_val || eq_sink_val || gt_sink_val,
                "Some set size must be allowed to exist");

    // Take care of the edge cases, where the construction below would collapse.
    if (set_size == 0) {
      if (!gt_sink_val) { return zdd_sink(eq_sink_val); }
    }

    adiar_debug(set_size > 0 || gt_sink_val,
                "Set size is only 0 if we accept a non-negative number of elements");

    if (set_size == 1) {
      if (!eq_sink_val && !gt_sink_val) { return zdd_null(); }
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
    const id_t max_id = set_size == 0              ? gt_sink_val    // gt_sink_val == 1
                      : set_size == 1              ? 2*gt_sink_val
                      : gt_sink_val && eq_sink_val ? set_size
                      : gt_sink_val                ? set_size + 1   // ~eq_sink
                      : eq_sink_val                ? set_size - 1   // ~gt_sink
                                                   : set_size - 2   // ~eq_sink /\ ~gt_sink
      ;

    const bool not_equal = lt_sink_val && !eq_sink_val && gt_sink_val;

    id_t prior_min_id = MAX_ID; // <-- dummy value to squelch the compiler

    size_t processed_levels = 0;
    label_t prior_label = MAX_LABEL; // <-- dummy value to squelch the compiler

    do {
      label_t curr_label = ls.pull();
      size_t level_size = 0;

      // Start with the maximal number the accumulated value can be at
      // up to this label.
      const size_t remaining_levels = labels_size - processed_levels - 1; // exclusive of current level
      id_t curr_id = std::min(remaining_levels, max_id);

      // How small has the accumulated sum up to this point to be, such that it
      // is still possible to reach the last node before the last label?
      const id_t curr_level_width = processed_levels
        // Add node for reached-equality/-greater on high
        + ((eq_sink_val && gt_sink_val) || gt_sink_val)
        // Add node for almost-never-reach-equality
        + (not_equal && processed_levels > 0)
        ;

      const id_t min_id = curr_level_width < max_id ? max_id - curr_level_width : 0;

      do {
        ptr_t low;
        if (processed_levels == 0) { // lowest level
          low = curr_id == set_size+1 ? gt_sink
              : curr_id == set_size   ? eq_sink
              : lt_sink;
        } else if (curr_id < prior_min_id) { // guaranteed to be in lt case
          if (not_equal) {
            low = curr_id == min_id
              ? create_node_ptr(prior_label, max_id)
              : lt_sink; // <- When processed_levels == 1 and happens twice
          } else {
            low = lt_sink_val
              ? create_node_ptr(prior_label, prior_min_id)
              : lt_sink;
          }
        } else {
          low = create_node_ptr(prior_label, curr_id);
        }

        ptr_t high;
        if (processed_levels == 0) {
          high = curr_id >= set_size   ? gt_sink
               : curr_id == set_size-1 ? eq_sink
               : lt_sink;
        } else if (curr_id == max_id) {
          high = gt_sink_val ? low
               : eq_sink_val ? eq_sink
               : lt_sink;
        } else if (not_equal && processed_levels == 1 && curr_id == min_id){
          high = lt_sink; // <-- true sink
        } else {
          high = create_node_ptr(prior_label, curr_id + 1);
        }

        adiar_debug(high != create_sink_ptr(false), "Should not create a reducible node");

        nw.unsafe_push(create_node(curr_label, curr_id, low, high));
        level_size++;
      } while (curr_id-- > min_id);

      adiar_debug(level_size > 0, "Should have output a node");
      nw.unsafe_push(create_meta(curr_label, level_size));

      prior_label = curr_label;
      prior_min_id = curr_id + 1;

      processed_levels++;
    } while (ls.can_pull());

    return nf;
  }
}

#endif // ADIAR_ZDD_BUILD_H

#include "pred.h"

#include <adiar/file_stream.h>

namespace adiar
{
  stats_t::equality_t stats_equality;

  //////////////////////////////////////////////////////////////////////////////
  // Slow O(sort(N)) I/Os comparison by traversing the product construction and
  // comparing each related pair of nodes.

  //////////////////////////////////////////////////////////////////////////////
  // Check whether more requests were processed on this level than allowed. An
  // isomorphic DAG would not create more requests than the original number of
  // nodes.
  template<bool tv>
  class input_bound_levels
  {
  private:
    level_info_stream<node_t, 1> in_meta_1;

    size_t curr_level_size;
    size_t curr_level_processed;

  public:
    input_bound_levels(const node_file &f1, const node_file &/*f2*/)
      : in_meta_1(f1),
        curr_level_size(size_of(in_meta_1.pull())),
        curr_level_processed(1)
    { }

    void next_level(label_t /* level */)
    { // Ignore input, since only used with the isomorphism_policy below.
      curr_level_size = size_of(in_meta_1.pull());
      curr_level_processed = 0;
    }

    bool on_step()
    {
      curr_level_processed++;
      const bool ret_value = curr_level_size < curr_level_processed;
#ifdef ADIAR_STATS
      if (ret_value) { stats_equality.slow_check.exit_on_processed_on_level++; }
#endif
      return ret_value;
    }

    static constexpr bool termination_value = tv;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Precondition:
  //  - The number of nodes are the same
  //  - The number of levels are the same
  //  - The label and size of each level are the same
  class isomorphism_policy : public prod_same_level_merger
  {
  public:
    typedef input_bound_levels<false> level_check_t;

  public:
    static bool resolve_sinks(const node_t &v1, const node_t &v2, bool &ret_value)
    {
      ret_value = is_sink(v1) && is_sink(v2) && value_of(v1) == value_of(v2);
#ifdef ADIAR_STATS
      stats_equality.slow_check.exit_on_root++;
#endif
      return true;
    }

  public:
    static bool resolve_singletons(const node &v1, const node_t v2)
    {
#ifdef ADIAR_STATS
      stats_equality.slow_check.exit_on_root++;
#endif
      adiar_debug(label_of(v1) == label_of(v2), "Levels match per the precondition");
      return v1.low == v2.low && v1.high == v2.high;
    }

  public:
    static bool resolve_request(comparison_priority_queue_t &pq,
                                ptr_t r1, ptr_t r2)
    {
      // Are they both a sink (and the same sink)?
      if (is_sink(r1) || is_sink(r2)) {
        if (is_sink(r1) && is_sink(r2) && value_of(r1) == value_of(r2)) {
          return false;
        } else {
#ifdef ADIAR_STATS
          stats_equality.slow_check.exit_on_children++;
#endif
          return true;
        }
      }

      // Do they NOT point to a node with the same level?
      if (label_of(r1) != label_of(r2)) {
#ifdef ADIAR_STATS
        stats_equality.slow_check.exit_on_children++;
#endif
        return true;
      }

      // No violation, so recurse
      pq.push({ r1, r2 });
      return false;
    }

  public:
    // Since we guarantee to be on the same level, then we merely provide a noop
    // (similar to the bdd_policy) for the cofactor.
    static inline void compute_cofactor([[maybe_unused]] bool on_curr_level, ptr_t &, ptr_t &)
    { adiar_invariant(on_curr_level, "No request have mixed levels"); }

  public:
    static constexpr bool early_return_value = false;
    static constexpr bool no_early_return_value = true;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Fast 2N/B I/Os comparison by comparing the i'th nodes numerically. This
  // requires, that the node_file is 'canonical' in the following sense:
  //
  // - For each level, the ids are decreasing from MAX_ID in increments of one.
  // - There are no duplicate nodes.
  // - Nodes within each level are sorted by the children (e.g. ordered first on
  //   'high', secondly on 'low').
  //
  // See Section 3.3 in 'Efficient Binary Decision Diagram Manipulation in
  // External Memory' on arXiv (v2 or newer) for an induction proof this is a
  // valid comparison.

  /////////////////////
  // Precondition:
  //  - The number of nodes are the same (to simplify the 'while' condition)
  //  - The node_files are both 'canonical'.
  //  - The negation flags given for both node_files agree (breaks canonicity)
  bool fast_isomorphism_check(const node_file &f1, const node_file &f2)
  {
#ifdef ADIAR_STATS
    stats_equality.fast_check.runs++;
#endif
    node_stream<> in_nodes_1(f1);
    node_stream<> in_nodes_2(f2);

    while (in_nodes_1.can_pull()) {
      adiar_debug(in_nodes_2.can_pull(), "The number of nodes should coincide");
      if (in_nodes_1.pull() != in_nodes_2.pull()) {
#ifdef ADIAR_STATS
        stats_equality.fast_check.exit_on_mismatch++;
#endif
        return false;
      }
    }
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////
  bool is_isomorphic(const node_file &f1, const node_file &f2,
                     bool negate1, bool negate2)
  {
    // Are they literally referring to the same underlying file?
    if (f1._file_ptr == f2._file_ptr) {
#ifdef ADIAR_STATS
      stats_equality.exit_on_same_file++;
#endif
      return negate1 == negate2;
    }

    // Are they trivially not the same, since they have different number of
    // nodes (in _files[0])?
    if (f1._file_ptr -> _files[0].size() != f2._file_ptr -> _files[0].size()) {
#ifdef ADIAR_STATS
      stats_equality.exit_on_nodecount++;
#endif
      return false;
    }

    // Are they trivially not the same, since they have different number of
    // levels (in the _level_info_file)?
    if (f1._file_ptr -> _level_info_file.size() != f2._file_ptr -> _level_info_file.size()) {
#ifdef ADIAR_STATS
      stats_equality.exit_on_varcount++;
#endif
      return false;
    }

    // Are they trivially not the same, since the labels or the size of each
    // level does not match?
    { // Create new scope to garbage collect the two meta_streams early
      level_info_stream<node_t, 1> in_meta_1(f1);
      level_info_stream<node_t, 1> in_meta_2(f2);

      while (in_meta_1.can_pull()) {
        adiar_debug(in_meta_2.can_pull(), "level_info files are same size");
        if (in_meta_1.pull() != in_meta_2.pull()) {
#ifdef ADIAR_STATS
          stats_equality.exit_on_levels_mismatch++;
#endif
          return false;
        }
      }
    }

    // TODO: Use 'fast_isomorphism_check' when there is only one node per level.

    // Compare their content to discern whether there exists an isomorphism
    // between them.
    if (f1._file_ptr -> canonical && f2._file_ptr -> canonical && negate1 == negate2) {
      return fast_isomorphism_check(f1, f2);
    } else {
      return comparison_check<isomorphism_policy>(f1, f2, negate1, negate2);
    }
  }

  bool is_isomorphic(const decision_diagram &a, const decision_diagram &b)
  {
    return is_isomorphic(a.file, b.file, a.negate, b.negate);
  }
}

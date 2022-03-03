#ifndef ADIAR_INTERNAL_PRED_H
#define ADIAR_INTERNAL_PRED_H

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>

#include <adiar/internal/decision_diagram.h>
#include <adiar/internal/levelized_priority_queue.h>
#include <adiar/internal/product_construction.h>
#include <adiar/internal/tuple.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// Struct to hold statistics for equality checking
  extern stats_t::equality_t stats_equality;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Given two node files, computes whether they are isomorphic; i.e.
  /// whether they are equivalent.
  ///
  /// Checks whether the two files are isomorphic, i.e. whether there is a
  /// structure-preserving mapping between f1 and f2. This assumes, that both
  /// files are of a unique reduced form.
  ///
  /// \param fi      The two files of nodes to compare.
  /// \param negatei Whether the nodes of fi should be read in negated form
  ///
  /// \return    Whether the two node_files represent equivalent graphs.
  //////////////////////////////////////////////////////////////////////////////
  bool is_isomorphic(const node_file &f1, const node_file &f2,
                     bool negate1 = false, bool negate2 = false);

  bool is_isomorphic(const decision_diagram &a, const decision_diagram &b);

  //////////////////////////////////////////////////////////////////////////////
  // Data structures
  typedef levelized_node_priority_queue<tuple, tuple_label, tuple_fst_lt,
                                        external_sorter, external_priority_queue,
                                        2>
  comparison_priority_queue_t;

  typedef tpie::priority_queue<tuple_data, tuple_snd_lt>
  comparison_data_priority_queue_t;

  //////////////////////////////////////////////////////////////////////////////
  /// Behaviour can be changed with the 'comp_policy'.
  ///
  /// - The 'resolve_sinks' function resolves the case of being given two sinks.
  ///
  /// - The 'resolve_request' function checks for early termination and places
  ///   new recursion requests in the priority queue if more recursions are needed.
  ///
  /// - If the constexpr 'request_capped_by_level_size' variable is set to true,
  ///   then the algorithm is guaranteed to only run in O(sort(N_1)) number of
  ///   I/Os.
  ///
  /// - The constexpr 'early_return_value' and 'no_early_return_value' change the
  ///   return value on early returns.
  ///
  /// This 'prod_policy' also should inherit (or provide) the general policy for
  /// the decision_diagram used (i.e. bdd_policy in bdd/bdd.h, zdd_policy in
  /// zdd/zdd.h and so on). This provides the following functions
  ///
  /// - compute_cofactor:
  ///   Used to change the low and high children retrieved from the input during
  ///   the product construction.
  //////////////////////////////////////////////////////////////////////////////
  template<typename comp_policy>
  bool comparison_check(const node_file &f1, const node_file &f2,
                        bool negate1, bool negate2)
  {
    node_stream<> in_nodes_1(f1, negate1);
    node_stream<> in_nodes_2(f2, negate2);

    node_t v1 = in_nodes_1.pull();
    node_t v2 = in_nodes_2.pull();

    if (is_sink(v1) || is_sink(v2)) {
      bool ret_value;
      if (comp_policy::resolve_sinks(v1, v2, ret_value)) {
        return ret_value;
      }
    }

    if (is_sink(v1.low) && is_sink(v1.high) && is_sink(v2.low) && is_sink(v2.high)) {
      return comp_policy::resolve_singletons(v1, v2);
    }

    // Set up priority queue for recursion
    tpie::memory_size_type available_memory = tpie::get_memory_manager().available();

    comparison_priority_queue_t comparison_pq_1({f1, f2},
                                                (available_memory * 3) / 4,
                                                std::numeric_limits<size_t>::max());

    // Check for violation on root children, or 'recurse' otherwise
    label_t level = label_of(fst(v1.uid, v2.uid));

    ptr_t low1, high1, low2, high2;
    comp_policy::merge_root(low1,high1, low2,high2, level, v1, v2);

    comp_policy::compute_cofactor(on_level(v1, level), low1, high1);
    comp_policy::compute_cofactor(on_level(v2, level), low2, high2);

    if (comp_policy::resolve_request(comparison_pq_1, low1, low2)
        || comp_policy::resolve_request(comparison_pq_1, high1, high2)) {
      return comp_policy::early_return_value;
    }

    // Initialise level checking
    typename comp_policy::level_check_t level_checker(f1,f2);

    comparison_data_priority_queue_t comparison_pq_2(available_memory / 4);

    while (!comparison_pq_1.empty() || !comparison_pq_2.empty()) {
      if (comparison_pq_1.empty_level() && comparison_pq_2.empty()) {
        comparison_pq_1.setup_next_level();

        level_checker.next_level(comparison_pq_1.current_level());
      }

      ptr_t t1, t2;
      bool with_data;
      ptr_t data_low = NIL, data_high = NIL;

      // Merge requests from comparison_pq_1 and comparison_pq_2
      if (comparison_pq_1.can_pull() && (comparison_pq_2.empty() ||
                                         fst(comparison_pq_1.top()) < snd(comparison_pq_2.top()))) {
        with_data = false;
        t1 = comparison_pq_1.top().t1;
        t2 = comparison_pq_1.top().t2;

        comparison_pq_1.pop();
      } else {
        with_data = true;
        t1 = comparison_pq_2.top().t1;
        t2 = comparison_pq_2.top().t2;

        data_low = comparison_pq_2.top().data_low;
        data_high = comparison_pq_2.top().data_high;

        comparison_pq_2.pop();
      }

      // Seek request partially in stream
      ptr_t t_seek = with_data ? snd(t1,t2) : fst(t1,t2);
      while (v1.uid < t_seek && in_nodes_1.can_pull()) {
        v1 = in_nodes_1.pull();
      }
      while (v2.uid < t_seek && in_nodes_2.can_pull()) {
        v2 = in_nodes_2.pull();
      }

      // Skip all requests to the same node
      while (comparison_pq_1.can_pull() && (comparison_pq_1.top().t1 == t1
                                            && comparison_pq_1.top().t2 == t2)) {
        comparison_pq_1.pull();
      }

      // Forward information across the level
      if (!with_data
          && !is_sink(t1) && !is_sink(t2) && label_of(t1) == label_of(t2)
          && (v1.uid != t1 || v2.uid != t2)) {
        node_t v0 = prod_from_1(t1,t2) ? v1 : v2;

        comparison_pq_2.push({ t1, t2, v0.low, v0.high });
        continue;
      }

      if (level_checker.on_step()) {
        return level_checker.termination_value;
      }

      ptr_t low1, high1, low2, high2;
      comp_policy::merge_data(low1,high1, low2,high2,
                              t1, t2, t_seek,
                              v1, v2,
                              data_low, data_high);

      label_t level = label_of(t_seek);
      comp_policy::compute_cofactor(on_level(t1, level), low1, high1);
      comp_policy::compute_cofactor(on_level(t2, level), low2, high2);

      if (comp_policy::resolve_request(comparison_pq_1, low1, low2)
          || comp_policy::resolve_request(comparison_pq_1, high1, high2)) {
        return comp_policy::early_return_value;
      }
    }

    return comp_policy::no_early_return_value;
  }

  template<typename comp_policy>
  bool comparison_check(const decision_diagram &a, const decision_diagram &b)
  {
    return comparison_check<comp_policy>(a.file, b.file, a.negate, b.negate);
  }
}

#endif // ADIAR_INTERNAL_PRED_H

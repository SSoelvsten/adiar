#include "isomorphism.h"

#include <adiar/file_stream.h>
#include <adiar/levelized_priority_queue.h>
#include <adiar/tuple.h>

namespace adiar
{

  //////////////////////////////////////////////////////////////////////////////
  // Slow O(sort(N)) I/Os comparison by traversing the product construction and
  // comparing each related pair of nodes.

  /////////////////////
  // Data structures
  typedef levelized_node_priority_queue<tuple, tuple_label, tuple_fst_lt, std::less<>, 2> isomorphism_priority_queue_t;
  typedef tpie::priority_queue<tuple_data, tuple_snd_lt> isomorphism_data_priority_queue_t;

  /////////////////////
  // Helper Functions

  // Returns whether one can do an early termination with false returned
  inline bool isomorphism_resolve_request(isomorphism_priority_queue_t &pq,
                                          ptr_t r1, ptr_t r2)
  {
    // Are they both a sink (and the same sink)?
    if (is_sink(r1) || is_sink(r2)) {
      return is_sink(r1) && is_sink(r2)
        ? value_of(r1) != value_of(r2)
        : true;
    }

    // Do they NOT point to a node with the same label?
    if (label_of(r1) != label_of(r2)) {
      return true;
    }

    // No violation, so recurse
    pq.push({ r1, r2 });
    return false;
  }

  /////////////////////
  // Precondition:
  //  - The number of nodes are the same
  //  - The number of levels are the same
  //  - The label and size of each level are the same
  bool slow_isomorphism_check(const node_file &f1, const node_file &f2,
                              bool negate1, bool negate2)
  {
    node_stream<> in_nodes_1(f1, negate1);
    node_stream<> in_nodes_2(f2, negate2);

    node_t v1 = in_nodes_1.pull();
    node_t v2 = in_nodes_2.pull();

    if (is_sink(v1) && is_sink(v2)) {
      return value_of(v1) == value_of(v2);
    }
    adiar_debug(!is_sink(v1) && !is_sink(v2), "The levels (and hence the sinkness) should coincide");
    adiar_debug(label_of(v1) == label_of(v2), "The levels (and hence the root) should coincide");

    if (!in_nodes_1.can_pull()) {
      adiar_debug(!in_nodes_1.can_pull(), "The number of nodes should coincide");
      return v1.low == v2.low && v1.high == v2.high;
    }

    // Set up priority queue for recursion
    tpie::memory_size_type available_memory = tpie::get_memory_manager().available();

    isomorphism_priority_queue_t isomorphism_pq_1({f1,f2},(available_memory * 3) / 4);

    // Check for violation on root children, or 'recurse' otherwise
    if (isomorphism_resolve_request(isomorphism_pq_1, v1.low, v2.low)) {
      return false;
    }

    if (isomorphism_resolve_request(isomorphism_pq_1, v1.high, v2.high)) {
      return false;
    }

    size_t curr_level_processed = 1;

    isomorphism_data_priority_queue_t isomorphism_pq_2(calc_tpie_pq_factor(available_memory / 4));

    while (isomorphism_pq_1.can_pull() || isomorphism_pq_1.has_next_level() || !isomorphism_pq_2.empty()) {
      if (!isomorphism_pq_1.can_pull() && isomorphism_pq_2.empty()) {
        isomorphism_pq_1.setup_next_level();
      }

      ptr_t t1, t2;
      bool with_data;
      ptr_t data_low = NIL, data_high = NIL;

      // Merge requests from isomorphism_pq_1 and isomorphism_pq_2
      if (isomorphism_pq_1.can_pull() && (isomorphism_pq_2.empty() ||
                                          fst(isomorphism_pq_1.top()) < snd(isomorphism_pq_2.top()))) {
        with_data = false;
        t1 = isomorphism_pq_1.top().t1;
        t2 = isomorphism_pq_1.top().t2;

        isomorphism_pq_1.pop();
      } else {
        with_data = true;
        t1 = isomorphism_pq_2.top().t1;
        t2 = isomorphism_pq_2.top().t2;

        data_low = isomorphism_pq_2.top().data_low;
        data_high = isomorphism_pq_2.top().data_high;

        isomorphism_pq_2.pop();
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
      while (isomorphism_pq_1.can_pull() && (isomorphism_pq_1.top().t1 == t1
                                             && isomorphism_pq_1.top().t2 == t2)) {
        isomorphism_pq_1.pull();
      }

      // Forward information across the level
      bool from_1 = fst(t1,t2) == t1;

      if (!with_data
          && !is_sink(t1) && !is_sink(t2) && label_of(t1) == label_of(t2)
          && (v1.uid != t1 || v2.uid != t2)) {
        node_t v0 = from_1 ? v1 : v2;

        isomorphism_pq_2.push({ t1, t2, v0.low, v0.high });
        continue;
      }

      // Check for violation in request, or 'recurse' otherwise
      if (isomorphism_resolve_request(isomorphism_pq_1,
                                      with_data && from_1 ? data_low : v1.low,
                                      with_data && !from_1 ? data_low : v2.low)) {
        return false;
      }

      if (isomorphism_resolve_request(isomorphism_pq_1,
                                      with_data && from_1 ? data_high : v1.high,
                                      with_data && !from_1 ? data_high : v2.high)) {
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
      return negate1 == negate2;
    }

    // Are they trivially not the same, since they have different number of
    // nodes (in _files[0])?
    if (f1._file_ptr -> _files[0].size() != f2._file_ptr -> _files[0].size()) {
      return false;
    }

    // Are they trivially not the same, since they have different number of
    // levels (in the _meta_file)?
    if (f1._file_ptr -> _meta_file.size() != f2._file_ptr -> _meta_file.size()) {
      return false;
    }

    // Are they trivially not the same, since the labels or the size of each
    // level does not match?
    { // Create new scope to garbage collect the two meta_streams early
      meta_stream<node_t, 1> in_meta_1(f1);
      meta_stream<node_t, 1> in_meta_2(f2);

      while (in_meta_1.can_pull()) {
        adiar_debug(in_meta_2.can_pull(), "meta files are same size");
        if (in_meta_1.pull() != in_meta_2.pull()) {
          return false;
        }
      }
    }

    return slow_isomorphism_check(f1, f2, negate1, negate2);
  }
}

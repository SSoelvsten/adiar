#ifndef ADIAR_INTERNAL_ALGORITHMS_NESTED_SWEEPING_H
#define ADIAR_INTERNAL_ALGORITHMS_NESTED_SWEEPING_H

#include <functional>

#include <adiar/statistics.h>

#include <adiar/internal/cut.h>
#include <adiar/internal/algorithms/reduce.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_structures/sorter.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/memory.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// Name space for helper functions and classes for the `nested_sweep`
  /// algorithm below.
  //////////////////////////////////////////////////////////////////////////////
  namespace nested_sweeping
  {
    //////////////////////////////////////////////////////////////////////////////
    /// Struct to hold statistics
    extern stats_t::nested_sweeping_t stats;

    //////////////////////////////////////////////////////////////////////////
    /// \brief Return type for PQ size computations.
    //////////////////////////////////////////////////////////////////////////
    struct pq_type_results
    {
    public:
      enum pq_choice_t { INTERNAL_PQ, INTERNAL_BUCKETED, EXTERNAL_BUCKETED };

    public:
      const pq_choice_t pq_choice;
      const size_t      max_pq_memory;
      const cut_size_t  max_pq_size;
    };

    namespace outer
    {
      //////////////////////////////////////////////////////////////////////////
      /// \brief Default priority queue for the Outer Up Sweep.
      //////////////////////////////////////////////////////////////////////////
      template<size_t LOOK_AHEAD, memory_mode_t mem_mode>
      using up__pq_t = reduce_priority_queue<LOOK_AHEAD, mem_mode>;

      //////////////////////////////////////////////////////////////////////////
      /// \brief Default policy for the Inner Up Sweep.
      //////////////////////////////////////////////////////////////////////////
      template<typename dd_policy>
      class up__policy_t : public dd_policy
      {
      public:
        template<size_t LOOK_AHEAD, memory_mode_t mem_mode>
        using pq_t = up__pq_t<LOOK_AHEAD, mem_mode>;
      };

      //////////////////////////////////////////////////////////////////////////
      /// \brief Decorator for the Reduce priority queue that either forwards
      ///        to itself or to a sorter, depending on the level of the source.
      ///
      /// \details This is to be used in the *outer* sweep when forwarding
      ///          information to a level above the current one.
      ///
      /// \sa pq_t, levelized_priority_queue, nested_sweep
      //////////////////////////////////////////////////////////////////////////
      template<typename outer_pq_t, typename inner_roots_t>
      class up__pq_decorator
      {
      public:
        ////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the elements.
        ////////////////////////////////////////////////////////////////////////////
        using elem_t = typename outer_pq_t::elem_t;

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the element comparator.
        ////////////////////////////////////////////////////////////////////////////
        using elem_comp_t = typename outer_pq_t::elem_comp_t;

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Number of buckets.
        ////////////////////////////////////////////////////////////////////////////
        static constexpr memory_mode_t mem_mode = outer_pq_t::memory_mode;

      private:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Reference to the outer sweep's (levelized) priority queue.
        ////////////////////////////////////////////////////////////////////////
        // TODO (optimisation): public inheritance?
        outer_pq_t &_outer_pq;

        ////////////////////////////////////////////////////////////////////////
        /// \brief Reference to the sorter in the outer sweep that contains the
        ///        root requests generated for the next inner sweep.
        ////////////////////////////////////////////////////////////////////////
        inner_roots_t &_inner_roots;

        ////////////////////////////////////////////////////////////////////////
        /// \brief Type of a level.
        ////////////////////////////////////////////////////////////////////////
        using level_t = typename elem_t::ptr_t::label_t;

        ////////////////////////////////////////////////////////////////////////
        /// \brief The level of the next inner sweep;
        ////////////////////////////////////////////////////////////////////////
        // TODO: turn into signed value to allow using decorator above last
        //       legal value of '_next_inner' (abusing negative numbers).
        const level_t _next_inner;

      public:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Instantiate decorator.
        ////////////////////////////////////////////////////////////////////////
        up__pq_decorator(outer_pq_t &outer_pq,
                         inner_roots_t &inner_roots,
                         const level_t next_inner)
          : _outer_pq(outer_pq)
          , _inner_roots(inner_roots)
          , _next_inner(next_inner)
        { }

      public:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Number of terminals (of each type) placed within the priority
        ///        queue.
        ////////////////////////////////////////////////////////////////////////
        size_t terminals(const bool terminal_value) const
        { return _outer_pq.terminals(terminal_value); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Numer of elements in both the priority queue and the sorter.
        ////////////////////////////////////////////////////////////////////////
        size_t size() const
        { return _outer_pq.size() + _inner_roots.size(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Total number of arcs (across all levels) ignoring terminals.
        ////////////////////////////////////////////////////////////////////////
        size_t size_without_terminals() const
        { return size() - terminals(false) - terminals(true); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether there are more elements to pull for this level.
        ////////////////////////////////////////////////////////////////////////
        bool can_pull() const
        { return _outer_pq.can_pull(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Obtain and remove the next element from this level.
        ////////////////////////////////////////////////////////////////////////
        reduce_arc pull()
        { return _outer_pq.pull(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Remove the top arc on the current level.
        ////////////////////////////////////////////////////////////////////////
        void pop()
        { _outer_pq.pull(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Adds an element to the priority queue or the sorter,
        ///        depending on the level. Terminals always stay in the priority
        ///        queue
        ////////////////////////////////////////////////////////////////////////
        void push(const reduce_arc &a)
        {
          // TODO: use outer_pq::level_comp_t instead of hardcoding '<'
          if (a.source().label() < _next_inner && !a.target().is_terminal()) {
            _inner_roots.push(a);
          } else {
            _outer_pq.push(a);
          }
        }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether the priority queue and the sorter are empty.
        ////////////////////////////////////////////////////////////////////////
        bool empty() const
        { return size() == 0u; }
      };
    } // namespace outer

    namespace inner
    {
      //////////////////////////////////////////////////////////////////////////
      /// \brief Sorter for root requests to inner sweep
      ///
      /// \details This is to be used as the bridge from the *outer* bottom-up
      ///          sweep to the *inner* top-down sweep.
      ///
      /// \sa sorter, nested_sweep
      //////////////////////////////////////////////////////////////////////////
      template<memory_mode_t memory_mode,
               typename element_t,
               typename element_comp_t>
      class roots_sorter
      {
      public:
        ////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the elements.
        ////////////////////////////////////////////////////////////////////////////
        using elem_t = element_t;

        static_assert(elem_t::inputs == 1 && elem_t::sorted_target,
                      "Request should be on a single input, and hence sorted");

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the element comparator.
        ////////////////////////////////////////////////////////////////////////////
        using elem_comp_t = element_comp_t;

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Number of buckets.
        ////////////////////////////////////////////////////////////////////////////
        static constexpr memory_mode_t mem_mode = memory_mode;

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the sorter.
        ////////////////////////////////////////////////////////////////////////////
        using sorter_t = sorter<mem_mode, elem_t, elem_comp_t>;

      private:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Resetabble sorter for elements
        ////////////////////////////////////////////////////////////////////////
        unique_ptr<sorter_t> _sorter_ptr;

        ////////////////////////////////////////////////////////////////////////
        // NOTE: There is not '_terminals[2]' like in the priority queue, since
        //       there is as an invariant that the target is never a terminal.

        ////////////////////////////////////////////////////////////////////////
        const size_t _memory_bytes;
        const size_t _no_arcs;

      public:
        ////////////////////////////////////////////////////////////////////////
        static tpie::memory_size_type
        memory_usage(tpie::memory_size_type no_elements)
        { return sorter_t::memory_usage(no_elements); }

        ////////////////////////////////////////////////////////////////////////
        static tpie::memory_size_type
        memory_fits(tpie::memory_size_type memory_bytes)
        { return sorter_t::memory_fits(memory_bytes); }

      public:
        ////////////////////////////////////////////////////////////////////////
        roots_sorter(size_t memory_bytes, size_t no_arcs)
          : _sorter_ptr(sorter_t::make_unique(memory_bytes, no_arcs))
          , _memory_bytes(memory_bytes)
          , _no_arcs(no_arcs)
        { }

      public:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Push request (marked as from outer sweep).
        ////////////////////////////////////////////////////////////////////////
        void push(const elem_t& e)
        {
          // TODO: support requests with more than just the source
          _sorter_ptr->push({ e.target, {}, {flag(e.data.source)} });
        }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Convert arc to request (and mark it as from outer sweep).
        ////////////////////////////////////////////////////////////////////////
        void push(const reduce_arc& a)
        {
          adiar_invariant(!a.target().is_terminal(),
                          "Arcs to terminals always reside in the outer PQ");

          // TODO: support requests with more than just the source

          // TODO: Is there a better way to explicitly set the remainders of
          //       target to NIL?

          if constexpr (elem_t::cardinality == 1u) {
            _sorter_ptr->push(elem_t({a.target()}, {}, {flag(a.source())}));
          } else if constexpr (elem_t::cardinality == 2u) {
            _sorter_ptr->push(elem_t({a.target(), elem_t::ptr_t::NIL()}, {}, {flag(a.source())}));
          } else {
            static_assert(elem_t::cardinality <= 2u,
                          "Missing implementation for larger than binary combinators");
          }
        }

        ////////////////////////////////////////////////////////////////////////
        void sort()
        { _sorter_ptr->sort(); }

        ////////////////////////////////////////////////////////////////////////
        bool can_pull() /*const*/
        { return _sorter_ptr->can_pull(); }

        ////////////////////////////////////////////////////////////////////////
        elem_t top() /*const*/
        { return _sorter_ptr->top(); }

        ////////////////////////////////////////////////////////////////////////
        elem_t pull()
        {
          // TODO: decrement terminal count (is this number ever relevant before
          //       it already is 'reset()'?)
          return _sorter_ptr->pull();
        }

        ////////////////////////////////////////////////////////////////////////
        void reset()
        { sorter_t::reset_unique(_sorter_ptr, _memory_bytes, _no_arcs); }

        ////////////////////////////////////////////////////////////////////////
        size_t size() /*const*/
        { return _sorter_ptr->size(); }

        bool empty() /*const*/
        { return _sorter_ptr->empty(); }
      };

      //////////////////////////////////////////////////////////////////////////
      /// \brief Decorator for a (levelized) priority queue that merges the
      ///        requests created within the inner sweep with the (sorted) list
      ///        of requests from outer sweep.
      ///
      /// \details This is to be used as the (levelized) priority queue for the
      ///          *inner* top-down sweep.
      ///
      /// \sa levelized_priority_queue, nested_sweep
      //////////////////////////////////////////////////////////////////////////
      template<typename inner_pq_t, typename inner_roots_t>
      class down__pq_decorator
      {
      public:
        ////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the elements.
        ////////////////////////////////////////////////////////////////////////////
        using elem_t = typename inner_pq_t::elem_t;

        // TODO: static_assert(inner_pq_t::elem_t == inner_roots_t::elem_t);

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the element comparator.
        ////////////////////////////////////////////////////////////////////////////
        using elem_comp_t = typename inner_pq_t::elem_comp_t;

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Number of buckets.
        ////////////////////////////////////////////////////////////////////////////
        static constexpr memory_mode_t mem_mode = inner_pq_t::memory_mode;

        ////////////////////////////////////////////////////////////////////////
        /// \brief Type of the elements in the priority queue / sorter.
        ////////////////////////////////////////////////////////////////////////
        using level_t = typename elem_t::ptr_t::label_t;

      private:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Reference to the inner sweep's (levelized) priority queue.
        ////////////////////////////////////////////////////////////////////////
        // TODO (optimisation): public inheritance?
        inner_pq_t &_inner_pq;

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Instantiation of the comparator between elements.
        ////////////////////////////////////////////////////////////////////////////
        elem_comp_t _e_comparator = elem_comp_t();

        ////////////////////////////////////////////////////////////////////////
        /// \brief Reference to the sorter in the outer sweep that contains the
        ///        root requests generated for the next inner sweep.
        ////////////////////////////////////////////////////////////////////////
        inner_roots_t &_inner_roots;

      public:
        ////////////////////////////////////////////////////////////////////////////
        /// \brief Value to reflect 'out of levels'.
        ////////////////////////////////////////////////////////////////////////////
        static constexpr level_t NO_LABEL = inner_pq_t::NO_LABEL;

      public:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Instantiate decorator.
        ////////////////////////////////////////////////////////////////////////
        down__pq_decorator(inner_pq_t &inner_pq, inner_roots_t &inner_roots)
          : _inner_pq(inner_pq)
          , _inner_roots(inner_roots)
        { }

      public:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether there is any current level to pull elements from.
        ////////////////////////////////////////////////////////////////////////
        bool has_current_level() const
        { return _inner_pq.has_current_level(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief The label of the current level.
        ////////////////////////////////////////////////////////////////////////
        level_t current_level() const
        { return _inner_pq.current_level(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether there are any more (possibly all empty) levels.
        ////////////////////////////////////////////////////////////////////////
        bool has_next_level() /*const*/
        { return _inner_pq.has_next_level(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief The label of the next (possibly empty) level.
        ////////////////////////////////////////////////////////////////////////
        level_t next_level() /*const*/
        { return _inner_pq.next_level(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether one can push elements.
        ////////////////////////////////////////////////////////////////////////
        bool can_push() /*const*/
        { return _inner_pq.can_push(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Push request to inner priority queue.
        ////////////////////////////////////////////////////////////////////////
        void push(const elem_t& e)
        { _inner_pq.push(e); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Set up the next nonempty level in the priority queue and the
        ///        sorter (down to the given `stop_level`).
        ////////////////////////////////////////////////////////////////////////
        void setup_next_level(level_t stop_level = NO_LABEL)
        {
          if (_inner_roots.can_pull()) {
            stop_level = std::min(stop_level, _inner_roots.top().level());
          }
          _inner_pq.setup_next_level(stop_level);
        }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether this level is empty of requests.
        ///
        /// \pre has_current_level
        ///
        /// \sa can_pull, empty
        ////////////////////////////////////////////////////////////////////////
        bool empty_level() const
        {
          return _inner_pq.empty_level()
            && (!_inner_roots.can_pull() || _inner_roots.top().level() != current_level());
        }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether there are more requests to pull for this level.
        ////////////////////////////////////////////////////////////////////////
        bool can_pull() const
        { return !empty_level(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the top request of this level.
        ///
        /// \pre `can_pull() == true`
        ////////////////////////////////////////////////////////////////////////
        elem_t top()
        { return pq_first() ? _inner_pq.top() : _inner_roots.top(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the top request of this level.
        ///
        /// \pre `can_pull() == true`
        ////////////////////////////////////////////////////////////////////////
        elem_t peek()
        { return top(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Obtain and remove the top request of this level.
        ///
        /// \pre `can_pull() == true`
        ////////////////////////////////////////////////////////////////////////
        elem_t pull()
        { return pq_first() ? _inner_pq.pull() : _inner_roots.pull(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Remove the top arc on the current level.
        ///
        /// \pre `can_pull() == true`
        ////////////////////////////////////////////////////////////////////////
        void pop()
        { pull(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief The number of requests in the priority queue and the sorter.
        ////////////////////////////////////////////////////////////////////////
        size_t size() /*const*/
        { return _inner_pq.size() + _inner_roots.size(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether the priority queue and the sorter are empty.
        ////////////////////////////////////////////////////////////////////////
        bool empty() /*const*/
        { return _inner_pq.empty() && _inner_roots.empty(); }

      private:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Predicate whether to take the next element from the priority
        ///        queue or not. If not, it should be take from the sorter.
        ////////////////////////////////////////////////////////////////////////
        bool pq_first()
        {
          if (_inner_pq.empty_level())  { return false; }
          if (!_inner_roots.can_pull()) { return true; }

          const level_t _inner_roots_level = _inner_roots.top().level();
          if (_inner_roots_level != current_level()) { return true; }

          return _e_comparator(_inner_pq.top(), _inner_roots.top());
        }
      };

      //////////////////////////////////////////////////////////////////////////
      /// \brief Start the nested sweep given initial recursions in
      ///        `inner_roots` on DAG in `outer_file`.
      //////////////////////////////////////////////////////////////////////////
      // TODO

      //////////////////////////////////////////////////////////////////////////
      /// \brief Default priority queue for the Inner Up Sweep.
      //////////////////////////////////////////////////////////////////////////
      template<size_t LOOK_AHEAD, memory_mode_t mem_mode>
      using up__pq_t = outer::up__pq_t<LOOK_AHEAD, mem_mode>;

      //////////////////////////////////////////////////////////////////////////
      /// \brief Default policy for the Inner Up Sweep.
      //////////////////////////////////////////////////////////////////////////
      template<typename dd_policy>
      using up__policy_t = outer::up__policy_t<dd_policy>;

      //////////////////////////////////////////////////////////////////////////
      /// \brief Decorator for a (levelized) priority queue that either forwards
      ///        it to an outer one or to itself depending on whether the node
      ///        is generated from the inner or the outer sweep.
      ///
      /// \details This is to be used as the (levelized) priority queue for the
      ///          *inner* bottom-down reduce sweep .
      ///
      /// \sa levelized_priority_queue, nested_sweep
      //////////////////////////////////////////////////////////////////////////
      template<typename inner_pq_t, typename outer_pq_t>
      class up__pq_decorator
      {
      public:
        ////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the elements.
        ////////////////////////////////////////////////////////////////////////////
        using elem_t = typename inner_pq_t::elem_t;

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the element comparator.
        ////////////////////////////////////////////////////////////////////////////
        using elem_comp_t = typename inner_pq_t::elem_comp_t;

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Number of buckets.
        ////////////////////////////////////////////////////////////////////////////
        static constexpr memory_mode_t mem_mode = inner_pq_t::memory_mode;

        ////////////////////////////////////////////////////////////////////////
        /// \brief Type of the elements in the priority queue / sorter.
        ////////////////////////////////////////////////////////////////////////
        using level_t = typename elem_t::ptr_t::label_t;

      private:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Reference to the inner sweep's (levelized) priority queue.
        ////////////////////////////////////////////////////////////////////////
        // TODO (optimisation): public inheritance?
        inner_pq_t &_inner_pq;

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Instantiation of the comparator between elements.
        ////////////////////////////////////////////////////////////////////////////
        elem_comp_t _e_comparator = elem_comp_t();

        ////////////////////////////////////////////////////////////////////////
        /// \brief Reference to the outer sweep's (levelized) priority queue.
        ////////////////////////////////////////////////////////////////////////
        outer_pq_t &_outer_pq;

      public:
        ////////////////////////////////////////////////////////////////////////////
        /// \brief Value to reflect 'out of levels'.
        ////////////////////////////////////////////////////////////////////////////
        static constexpr level_t NO_LABEL = inner_pq_t::NO_LABEL;

      public:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Instantiate decorator.
        ////////////////////////////////////////////////////////////////////////
        up__pq_decorator(inner_pq_t &inner_pq, outer_pq_t &outer_pq)
          : _inner_pq(inner_pq)
          , _outer_pq(outer_pq)
        { }

      public:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether there is any current level to pull elements from.
        ////////////////////////////////////////////////////////////////////////
        bool has_current_level() const
        { return _inner_pq.has_current_level(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief The label of the current level.
        ////////////////////////////////////////////////////////////////////////
        level_t current_level() const
        { return _inner_pq.current_level(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether there are any more (possibly all empty) levels.
        ////////////////////////////////////////////////////////////////////////
        bool has_next_level() /*const*/
        { return _inner_pq.has_next_level(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief The label of the next (possibly empty) level.
        ////////////////////////////////////////////////////////////////////////
        level_t next_level() /*const*/
        { return _inner_pq.next_level(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether one can push elements.
        ////////////////////////////////////////////////////////////////////////
        bool can_push() const
        { return true; }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Push request to inner priority queue.
        ////////////////////////////////////////////////////////////////////////
        void push(const elem_t& e)
        {
          if (e.source().is_flagged()) {
            _outer_pq.push(arc(unflag(e.source()), e.target()));
          } else {
            _inner_pq.push(e);
          }
        }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Set up the next nonempty level in the priority queue and the
        ///        sorter (down to the given `stop_level`).
        ////////////////////////////////////////////////////////////////////////
        void setup_next_level(level_t stop_level = NO_LABEL)
        { _inner_pq.setup_next_level(stop_level); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether this level is empty of requests.
        ///
        /// \pre has_current_level
        ///
        /// \sa can_pull, empty
        ////////////////////////////////////////////////////////////////////////
        bool empty_level() const
        { return _inner_pq.empty_level(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether there are more requests to pull for this level.
        ////////////////////////////////////////////////////////////////////////
        bool can_pull() const
        { return _inner_pq.can_pull(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the top request of this level.
        ///
        /// \pre `can_pull() == true`
        ////////////////////////////////////////////////////////////////////////
        elem_t top()
        { return _inner_pq.top(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the top request of this level.
        ///
        /// \pre `can_pull() == true`
        ////////////////////////////////////////////////////////////////////////
        elem_t peek()
        { return _inner_pq.peek(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Obtain and remove the top request of this level.
        ///
        /// \pre `can_pull() == true`
        ////////////////////////////////////////////////////////////////////////
        elem_t pull()
        { return _inner_pq.pull(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Remove the top arc on the current level.
        ///
        /// \pre `can_pull() == true`
        ////////////////////////////////////////////////////////////////////////
        void pop()
        { _inner_pq.pop(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Number of terminals (of each type) placed within either of
        ///        the priority queues.
        ////////////////////////////////////////////////////////////////////////
        size_t terminals(const bool terminal_value) const
        {
          return _outer_pq.terminals(terminal_value)
               + _inner_pq.terminals(terminal_value);
        }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Numer of elements in both priority queues.
        ////////////////////////////////////////////////////////////////////////
        size_t size() const
        {
          return _outer_pq.size() + _inner_pq.size();
        }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Total number of arcs (across all levels) ignoring terminals
        ///        in both priority queues.
        ////////////////////////////////////////////////////////////////////////
        size_t size_without_terminals() const
        {
          return _outer_pq.size_without_terminals()
               + _inner_pq.size_without_terminals();
        }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether the priority queue and the sorter are empty.
        ////////////////////////////////////////////////////////////////////////
        bool empty() /*const*/
        { return _inner_pq.empty(); }
      };

      //////////////////////////////////////////////////////////////////////////
      /// \brief Execute the Inner Up Sweep (part 2).
      ///
      /// \sa nested_sweep
      //////////////////////////////////////////////////////////////////////////
      template<typename inner_up_sweep, typename inner_pq_t, typename outer_pq_t>
      inline cuts_t
      up(const typename inner_up_sweep::shared_arcs_t &inner_unreduced,
         outer_pq_t &outer_pq,
         node_writer &outer_writer,
         const size_t inner_pq_memory,
         const size_t inner_pq_max_size,
         const size_t inner_sorters_memory)
      {
        // Set up input
        arc_stream<> inner_arcs(inner_unreduced);
        level_info_stream<> inner_levels(inner_unreduced);

        // Set up (decorated) priority queue
        inner_pq_t inner_pq({inner_unreduced}, inner_pq_memory, inner_pq_max_size);

        using decorator_t = up__pq_decorator<inner_pq_t, outer_pq_t>;
        decorator_t decorated_pq(inner_pq, outer_pq);

        // Run Reduce
        return __reduce<inner_up_sweep>(inner_arcs, inner_levels,
                                        decorated_pq,
                                        outer_writer,
                                        inner_sorters_memory);

        // TODO (optimisation):
        //   Since the 1-level cut is up to this (processed) level, we also know
        //   that the already globally counted arcs can be added on-top of the
        //   current maximum and then be forgotten.
        //
        //   This would (soundly?) decrease the over-approximation.
      }

      //////////////////////////////////////////////////////////////////////////
      /// \brief Execute the Inner Up Sweep (part 1).
      ///
      /// \sa nested_sweep
      //////////////////////////////////////////////////////////////////////////
      template<typename inner_up_sweep, typename outer_pq_t>
      cuts_t
      up(const typename inner_up_sweep::shared_arcs_t &inner_unreduced,
         outer_pq_t &outer_pq,
         node_writer &outer_writer,
         const size_t inner_memory)
      {
        // Compute amount of memory available for auxiliary data structures after
        // having opened all streams.
        //
        // We then may derive an upper bound on the size of auxiliary data
        // structures and check whether we can run them with a faster internal
        // memory variant.

        const size_t inner_aux_available_memory =
          inner_memory - arc_stream<>::memory_usage() - level_info_stream<>::memory_usage();

        const size_t inner_pq_memory = inner_aux_available_memory / 2;
        const size_t inner_sorters_memory =
          inner_aux_available_memory - inner_pq_memory - tpie::file_stream<mapping>::memory_usage();

        const tpie::memory_size_type inner_pq_memory_fits =
          inner_up_sweep::template pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode_t::INTERNAL>::memory_fits(inner_pq_memory);

        const size_t inner_pq_bound = inner_unreduced->max_1level_cut;

        const size_t inner_pq_max_size = memory_mode == memory_mode_t::INTERNAL
          ? std::min(inner_pq_memory_fits, inner_pq_bound)
          : inner_pq_bound;

        const bool external_only = memory_mode == memory_mode_t::EXTERNAL;
        if (!external_only && inner_pq_max_size <= no_lookahead_bound(1)) {
#ifdef ADIAR_STATS
          stats.inner.up.lpq.unbucketed += 1u;
#endif
          using inner_pq_t = typename inner_up_sweep::template pq_t<0, memory_mode_t::INTERNAL>;
          return up<inner_up_sweep, inner_pq_t>(inner_unreduced, outer_pq, outer_writer,
                                                inner_pq_memory, inner_pq_max_size, inner_sorters_memory);

        } else if(!external_only && inner_pq_max_size <= inner_pq_memory_fits) {
#ifdef ADIAR_STATS
          stats.inner.up.lpq.internal += 1u;
#endif
          using inner_pq_t = typename inner_up_sweep::template pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode_t::INTERNAL>;
          return up<inner_up_sweep, inner_pq_t>(inner_unreduced, outer_pq, outer_writer,
                                                inner_pq_memory, inner_pq_max_size, inner_sorters_memory);
        } else {
#ifdef ADIAR_STATS
          stats.inner.up.lpq.external += 1u;
#endif
          using inner_pq_t = typename inner_up_sweep::template pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode_t::EXTERNAL>;
          return up<inner_up_sweep, inner_pq_t>(inner_unreduced, outer_pq, outer_writer,
                                                inner_pq_memory, inner_pq_max_size, inner_sorters_memory);
        }
      }
    } // namespace inner
  } // namespace nested_sweeping

  //////////////////////////////////////////////////////////////////////////////
  // TODO: nested sweep algorithm
}

#endif // ADIAR_INTERNAL_ALGORITHMS_NESTED_SWEEPING_H

#ifndef ADIAR_INTERNAL_ALGORITHMS_NESTED_SWEEPING_H
#define ADIAR_INTERNAL_ALGORITHMS_NESTED_SWEEPING_H

#include <functional>

#include <adiar/statistics.h>

#include <adiar/internal/cut.h>
#include <adiar/internal/dd_func.h>
#include <adiar/internal/algorithms/reduce.h>
#include <adiar/internal/data_structures/levelized_priority_queue.h>
#include <adiar/internal/data_structures/sorter.h>
#include <adiar/internal/data_types/ptr.h>
#include <adiar/internal/data_types/request.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/util.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// Name space for helper functions and classes for the `nested_sweep`
  /// algorithm below.
  //////////////////////////////////////////////////////////////////////////////
  namespace nested_sweeping
  {
    ////////////////////////////////////////////////////////////////////////////
    /// Struct to hold statistics
    extern stats_t::nested_sweeping_t stats;

    ////////////////////////////////////////////////////////////////////////////
    // A fast non-canonical `reduce_level`
    //
    // Enum with values:
    //
    // - ALWAYS_CANONICAL /* Always use the classic `reduce_level` */
    //
    // - FINAL_CANONICAL  /* Use the faster but non-canonical version as long as
    //                       there is at least one Inner Down Sweep still to be
    //                       done later. */
    //
    // - NEVER_CANONICAL  /* Always use the faster but non-canonical version */
    //
    // - AUTO             /* Quite similar to FINAL_CANONICAL, but it may use
    //                       the slower version of `reduce_level` if we somehow
    //                       have reason to believe it will remove nodes, e.g.
    //                       the level is quite wide. */
    ////////////////////////////////////////////////////////////////////////////
    // TODO: enum
    // TODO: `__reduce_level__fast`

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Algorithms and Decorators for the Outer sweep.
    ////////////////////////////////////////////////////////////////////////////
    namespace outer
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
        static constexpr size_t DATA_STRUCTURES = 1u;

        ////////////////////////////////////////////////////////////////////////
        /// \brief Type of the elements.
        ////////////////////////////////////////////////////////////////////////
        using elem_t = element_t;

        static_assert(elem_t::inputs == 1 && elem_t::sorted_target,
                      "Request should be on a single input, and hence sorted");

        ////////////////////////////////////////////////////////////////////////
        /// \brief Type of the element comparator.
        ////////////////////////////////////////////////////////////////////////
        using elem_comp_t = element_comp_t;

        ////////////////////////////////////////////////////////////////////////
        /// \brief Memory mode of sorter.
        ////////////////////////////////////////////////////////////////////////
        static constexpr memory_mode_t mem_mode = memory_mode;

        ////////////////////////////////////////////////////////////////////////
        /// \brief Type of the sorter.
        ////////////////////////////////////////////////////////////////////////
        using sorter_t = sorter<mem_mode, elem_t, elem_comp_t>;

        ////////////////////////////////////////////////////////////////////////
        static constexpr typename elem_t::label_t NO_LEVEL =
          static_cast<typename elem_t::label_t>(-1);

      private:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Resetabble sorter for elements
        ////////////////////////////////////////////////////////////////////////
        unique_ptr<sorter_t> _sorter_ptr;

        ////////////////////////////////////////////////////////////////////////
        /// \brief Maximum source seen
        ////////////////////////////////////////////////////////////////////////
        typename elem_t::ptr_t _max_source = elem_t::ptr_t::NIL();

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
        /// \brief Whether a request can be pushed.
        ////////////////////////////////////////////////////////////////////////
        bool can_push()
        {
          return _sorter_ptr->can_push();
        }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Push request (marked as from outer sweep).
        ////////////////////////////////////////////////////////////////////////
        void push(const elem_t& e)
        {
          adiar_invariant(e.target.fst().is_node(),
                          "Requests should have at least one internal node");

          _max_source = _max_source.is_nil()
            ? e.data.source
            : std::max(_max_source, e.data.source);

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
            push(elem_t({a.target()}, {}, {flag(a.source())}));
          } else if constexpr (elem_t::cardinality == 2u) {
            push(elem_t({a.target(), elem_t::ptr_t::NIL()}, {}, {a.source()}));
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
        {
          sorter_t::reset_unique(_sorter_ptr, _memory_bytes, _no_arcs);
          _max_source = elem_t::ptr_t::NIL();
        }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Take ownership of another root sorter's content.
        ///
        /// \details This leaves the other in an illegal state that requires a
        ///          call to `reset()`.
        ////////////////////////////////////////////////////////////////////////
        void move(roots_sorter<memory_mode, element_t, element_comp_t> &o)
        {
          _sorter_ptr = std::move(o._sorter_ptr);
          _max_source = o._max_source;
        }

        ////////////////////////////////////////////////////////////////////////
        size_t size() /*const*/
        { return _sorter_ptr->size(); }

        bool empty() /*const*/
        { return _sorter_ptr->empty(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Level of the deepest source
        ////////////////////////////////////////////////////////////////////////
        typename elem_t::label_t deepest_source()
        { return _max_source.is_nil() ? NO_LEVEL : _max_source.label(); }
      };

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
      template<typename outer_pq_t, typename outer_roots_t>
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
        /// \brief Memory mode (same as decorated priority queue).
        ////////////////////////////////////////////////////////////////////////////
        static constexpr memory_mode_t mem_mode = outer_pq_t::mem_mode;

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
        outer_roots_t &_outer_roots;

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
        ////////////////////////////////////////////////////////////////////////////
        /// \brief Value to reflect 'out of levels'.
        ////////////////////////////////////////////////////////////////////////////
        static constexpr level_t NO_LABEL = outer_pq_t::NO_LABEL;

      public:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Instantiate decorator.
        ////////////////////////////////////////////////////////////////////////
        up__pq_decorator(outer_pq_t &outer_pq,
                         outer_roots_t &outer_roots,
                         const level_t next_inner)
          : _outer_pq(outer_pq)
          , _outer_roots(outer_roots)
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
        { return _outer_pq.size() + _outer_roots.size(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Total number of arcs (across all levels) ignoring terminals.
        ////////////////////////////////////////////////////////////////////////
        size_t size_without_terminals() const
        { return size() - terminals(false) - terminals(true); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether the current level is empty.
        ////////////////////////////////////////////////////////////////////////
        bool empty_level() const
        { return _outer_pq.empty_level(); }

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
        /// \brief Obtain the next element from this level.
        ////////////////////////////////////////////////////////////////////////
        reduce_arc top()
        { return _outer_pq.top(); }

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
          if (a.source().is_nil() || (a.source().label() < _next_inner && a.target().is_node())) {
            _outer_roots.push(a);
          } else {
            _outer_pq.push(a);
          }
        }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Push requests created at `_next_inner` to the priority queue
        ///        if it only has terminals; otherwise pushes to roots for
        ///        nested sweep.
        ///
        /// \param e The request with some `target` and `data.source`
        ///
        /// \pre   The request's source must be above `_next_inner` or `NIL`.
        ///
        /// \see request, request_with_data
        ////////////////////////////////////////////////////////////////////////
        void push(const typename outer_roots_t::elem_t &e)
        {
          adiar_precondition(e.data.source.is_nil()
                             || e.data.source.label() < _next_inner);
          if (e.target.fst().is_terminal()) {
            _outer_pq.push({ e.data.source, e.target.fst() });
          } else {
            _outer_roots.push(e);
          }
        }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Set up the next nonempty level in the priority queue and the
        ///        sorter (down to the given `stop_level`).
        ////////////////////////////////////////////////////////////////////////
        void setup_next_level(level_t stop_level = NO_LABEL)
        {
          _outer_pq.setup_next_level(stop_level);
          adiar_debug(_next_inner <= _outer_pq.current_level(),
                      "Outer PQ stays below next Inner Level");
        }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether the priority queue and the sorter are empty.
        ////////////////////////////////////////////////////////////////////////
        bool empty() const
        { return size() == 0u; }
      };

      //////////////////////////////////////////////////////////////////////////
      /// \brief Class to obtain from the Inner Down Sweep what is the next
      ///        level (ahead of the current one being processed).
      //////////////////////////////////////////////////////////////////////////
      template<typename inner_down_sweep>
      class inner_iterator
      {
      public:
        using level_t = typename inner_down_sweep::ptr_t::label_t;

      public:
        ////////////////////////////////////////////////////////////////////////
        static tpie::memory_size_type
        memory_usage()
        { return level_info_stream<>::memory_usage(); }

      private:
        level_info_stream<> _lis;
        inner_down_sweep    &_inner_impl;

      public:
        inner_iterator(const typename inner_down_sweep::shared_arcs_t &dag,
                       inner_down_sweep &inner_impl)
          : _lis(dag)
          , _inner_impl(inner_impl)
        { }

      public:
        static constexpr level_t NONE = static_cast<level_t>(-1);

        ////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the next level to do an inner sweep to pull.
        ///
        /// \returns The next inner level that should be recursed on (or `NONE`
        ///          if none are left)
        ////////////////////////////////////////////////////////////////////////
        level_t next_inner()
        {
          while (_lis.can_pull()) {
            const level_t l = _lis.pull().level();
            if (_inner_impl.has_sweep(l)) { return l; }
          }
          return NONE;
        }
      };
    } // namespace outer

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Algorithms and Decorators for the Inner sweeps.
    ////////////////////////////////////////////////////////////////////////////
    namespace inner
    {
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
      template<typename inner_pq_t, typename outer_roots_t>
      class down__pq_decorator
      {
      public:
        ////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the elements.
        ////////////////////////////////////////////////////////////////////////////
        using elem_t = typename inner_pq_t::elem_t;

        // TODO: static_assert(inner_pq_t::elem_t == outer_roots_t::elem_t);

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Type of the element comparator.
        ////////////////////////////////////////////////////////////////////////////
        using elem_comp_t = typename inner_pq_t::elem_comp_t;

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Memory mode (same as decorated priority queue).
        ////////////////////////////////////////////////////////////////////////////
        static constexpr memory_mode_t mem_mode = inner_pq_t::mem_mode;

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
        outer_roots_t &_outer_roots;

      public:
        ////////////////////////////////////////////////////////////////////////////
        /// \brief Value to reflect 'out of levels'.
        ////////////////////////////////////////////////////////////////////////////
        static constexpr level_t NO_LABEL = inner_pq_t::NO_LABEL;

      public:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Instantiate decorator.
        ////////////////////////////////////////////////////////////////////////
        down__pq_decorator(inner_pq_t &inner_pq, outer_roots_t &outer_roots)
          : _inner_pq(inner_pq)
          , _outer_roots(outer_roots)
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
          if (_outer_roots.can_pull()) {
            stop_level = std::min(stop_level, _outer_roots.top().level());
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
            && (!_outer_roots.can_pull() || _outer_roots.top().level() != current_level());
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
        {
          return __pq_first()
            ? _inner_pq.top()
            : __essential(_outer_roots.top());
        }

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
        {
          return __pq_first()
            ? _inner_pq.pull()
            : __essential(_outer_roots.pull());
        }

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
        { return _inner_pq.size() + _outer_roots.size(); }

        ////////////////////////////////////////////////////////////////////////
        /// \brief Whether the priority queue and the sorter are empty.
        ////////////////////////////////////////////////////////////////////////
        bool empty() /*const*/
        { return _inner_pq.empty() && _outer_roots.empty(); }

      private:
        ////////////////////////////////////////////////////////////////////////
        /// \brief Predicate whether to take the next element from the priority
        ///        queue or not. If not, it should be take from the sorter.
        ////////////////////////////////////////////////////////////////////////
        bool __pq_first()
        {
          if (_inner_pq.empty_level())  { return false; }
          if (!_outer_roots.can_pull()) { return true; }

          const level_t _outer_roots_level = _outer_roots.top().level();
          if (_outer_roots_level != current_level()) { return true; }

          return _e_comparator(_inner_pq.top(), _outer_roots.top());
        }

        elem_t __essential(const elem_t &r)
        {
          // TODO: generalize into a `map(r.target, essential)` in
          //       <adiar/internal/data_types/tuple.h>. Ensure everything can be
          //       inlined at compile-time.
          static_assert(elem_t::cardinality <= 2,
                        "No need to support higher cardinality (yet)");

          typename elem_t::target_t r_tgt;
          if constexpr (elem_t::cardinality == 1) {
            r_tgt = typename elem_t::target_t(essential(r.target[0]));
          } else {// if constexpr (elem_t::cardinality == 2) {
            r_tgt = typename elem_t::target_t(essential(r.target[0]),
                                              essential(r.target[1]));
          }
          return elem_t(r_tgt, {}, r.data);
        }
      };

      //////////////////////////////////////////////////////////////////////////
      /// \brief Start the nested sweep given initial recursions in
      ///        `outer_roots` on DAG in `outer_file`.
      ///
      /// \par Side Effects:
      /// Sorts and resets the given `outer_roots` sorter.
      //////////////////////////////////////////////////////////////////////////
      template<typename inner_down_sweep, typename outer_roots_t>
      typename inner_down_sweep::unreduced_t
      down(inner_down_sweep &inner_impl,
           const typename inner_down_sweep::shared_nodes_t &outer_file,
           outer_roots_t &outer_roots,
           const size_t inner_memory)
      {
        adiar_debug(outer_roots.size() > 0,
                    "Nested Sweep needs one or more roots");

        outer_roots.sort();

        // Since we do not know anything about inner_impl, we cannot be sure of
        // what type for its priority queue is optimal at this point in time.
        // For example, it might choose to use random access which changes the
        // sorting predicate (and hence the type).
        //
        // To avoid having to do the boiler-plate yourself, use
        // `down__sweep_switch` below (assuming your algorithm fits).
        const typename inner_down_sweep::unreduced_t res =
          inner_impl.sweep(outer_file, outer_roots, inner_memory);

        outer_roots.reset();
        return res;
      }

      //////////////////////////////////////////////////////////////////////////
      /// \brief Implements the default boiler-plate behaviour in the Inner Down
      ///        Sweep to (1) switch based on `memory_mode` and (2) decorate the
      ///        priority queue and the `roots_sorter`.
      //////////////////////////////////////////////////////////////////////////
      template<typename inner_down_sweep, typename outer_roots_t>
      inline typename inner_down_sweep::unreduced_t
      down__sweep_switch(inner_down_sweep &inner_impl,
                         const typename inner_down_sweep::shared_nodes_t &outer_file,
                         outer_roots_t &outer_roots,
                         const size_t inner_memory)
      {
        // ---------------------------------------------------------------------

        // Compute amount of memory available for auxiliary data structures after
        // having opened all streams.
        //
        // We then may derive an upper bound on the size of auxiliary data
        // structures and check whether we can run them with a faster internal
        // memory variant.
        const size_t inner_stream_memory = inner_down_sweep::stream_memory();
        adiar_debug(inner_stream_memory <= inner_memory,
                    "There should be enough memory to include all streams");

        // ---------------------------------------------------------------------
        // Case: Run Inner Sweep (with random access)
        // TODO

        // ---------------------------------------------------------------------
        // Case: Run Inner Sweep (with priority queues)

        const size_t inner_pq_memory = inner_impl.pq_memory(inner_memory - inner_stream_memory);
        adiar_debug(inner_pq_memory <= inner_memory - inner_stream_memory,
                    "There should be enough memory to include all streams and priority queue");

        const size_t inner_remaining_memory = inner_memory - inner_stream_memory - inner_pq_memory;

        const size_t inner_pq_fits =
          inner_down_sweep::template pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode_t::INTERNAL>::memory_fits(inner_pq_memory);

        const size_t inner_pq_bound = inner_impl.pq_bound(outer_file, outer_roots.size());

        const bool external_only = memory_mode == memory_mode_t::EXTERNAL;

        const size_t inner_pq_max_size = memory_mode == memory_mode_t::INTERNAL
          ? std::min(inner_pq_fits, inner_pq_bound)
          : inner_pq_bound;

        // TODO (bdd_compose): ask 'inner_down_sweep' implementation for the initalizer list
        if(!external_only && inner_pq_max_size <= no_lookahead_bound(outer_roots_t::elem_t::cardinality)) {
#ifdef ADIAR_STATS
          stats.inner.down.lpq.unbucketed += 1u;
#endif
          adiar_debug(inner_pq_max_size <= inner_pq_fits,
                      "'no_lookahead' implies it should (in practice) satisfy the '<='");

          using inner_pq_t = typename inner_down_sweep::template pq_t<0, memory_mode_t::INTERNAL>;
          inner_pq_t inner_pq({typename inner_down_sweep::reduced_t(outer_file)},
                              inner_pq_memory, inner_pq_max_size,
                              stats.inner.down.lpq);

          using decorator_t = down__pq_decorator<inner_pq_t, outer_roots_t>;
          decorator_t decorated_pq(inner_pq, outer_roots);

          return inner_impl.sweep_pq(outer_file, decorated_pq, inner_remaining_memory);
        } else if(!external_only && inner_pq_max_size <= inner_pq_fits) {
#ifdef ADIAR_STATS
          stats.inner.down.lpq.internal += 1u;
#endif
          using inner_pq_t = typename inner_down_sweep::template pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode_t::INTERNAL>;
          inner_pq_t inner_pq({typename inner_down_sweep::reduced_t(outer_file)},
                              inner_pq_memory, inner_pq_max_size,
                              stats.inner.down.lpq);

          using decorator_t = down__pq_decorator<inner_pq_t, outer_roots_t>;
          decorator_t decorated_pq(inner_pq, outer_roots);

          return inner_impl.sweep_pq(outer_file, decorated_pq, inner_remaining_memory);
        } else {
#ifdef ADIAR_STATS
          stats.inner.down.lpq.external += 1u;
#endif
          using inner_pq_t = typename inner_down_sweep::template pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode_t::EXTERNAL>;
          inner_pq_t inner_pq({typename inner_down_sweep::reduced_t(outer_file)},
                              inner_pq_memory, inner_pq_max_size,
                              stats.inner.down.lpq);

          using decorator_t = down__pq_decorator<inner_pq_t, outer_roots_t>;
          decorator_t decorated_pq(inner_pq, outer_roots);

          return inner_impl.sweep_pq(outer_file, decorated_pq, inner_remaining_memory);
        }
      }

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
        /// \brief Memory mode (same as decorated priority queue).
        ////////////////////////////////////////////////////////////////////////////
        static constexpr memory_mode_t mem_mode = inner_pq_t::mem_mode;

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
        /// \brief Number of elements in both priority queues.
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
      /// \brief ...
      //////////////////////////////////////////////////////////////////////////
      class up__arc_stream__decorator
      {
      private:
        arc_stream<> &_inner_arcs;
        const arc_stream<> &_outer_arcs;

      public:
        up__arc_stream__decorator(arc_stream<> &inner_arcs,
                                  const arc_stream<> &outer_arcs)
          : _inner_arcs(inner_arcs), _outer_arcs(outer_arcs)
        { }

      public:
        ////////////////////////////////////////////////////////////////////////////
        /// \brief Whether the stream contains more internal arcs.
        ////////////////////////////////////////////////////////////////////////////
        bool can_pull_internal() const
        { return _inner_arcs.can_pull_internal(); }

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the next internal arc (and move the read head).
        ////////////////////////////////////////////////////////////////////////////
        const arc pull_internal()
        { return _inner_arcs.pull_internal(); }

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the next internal arc (but do not move the read head).
        ////////////////////////////////////////////////////////////////////////////
        const arc peek_internal()
        { return _inner_arcs.peek_internal(); }

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the number of unread terminals.
        ////////////////////////////////////////////////////////////////////////////
        size_t unread_terminals() const
        { return _outer_arcs.unread_terminals() + _inner_arcs.unread_terminals(); }

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the number of unread terminals of a specific value.
        ////////////////////////////////////////////////////////////////////////////
        size_t unread_terminals(const bool terminal_value) const
        {
          return _outer_arcs.unread_terminals(terminal_value)
               + _inner_arcs.unread_terminals(terminal_value);
        }

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Whether the stream contains more terminal arcs.
        ////////////////////////////////////////////////////////////////////////////
        bool can_pull_terminal() const
        { return _inner_arcs.can_pull_terminal(); }

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the next arc (and move the read head).
        ////////////////////////////////////////////////////////////////////////////
        const arc pull_terminal()
        { return _inner_arcs.pull_terminal(); }

        ////////////////////////////////////////////////////////////////////////////
        /// \brief Obtain the next arc (but do not move the read head).
        ////////////////////////////////////////////////////////////////////////////
        const arc peek_terminal()
        { return _inner_arcs.peek_terminal(); }
      };

      //////////////////////////////////////////////////////////////////////////
      /// \brief Execute the Inner Up Sweep (part 2).
      ///
      /// \sa nested_sweep
      //////////////////////////////////////////////////////////////////////////
      template<typename inner_up_sweep, typename inner_pq_t, typename outer_pq_t>
      inline void
      up(const arc_stream<> &outer_arcs,
         outer_pq_t &outer_pq,
         node_writer &outer_writer,
         const typename inner_up_sweep::shared_arcs_t &inner_arcs_file,
         const size_t inner_pq_memory,
         const size_t inner_pq_max_size,
         const size_t inner_sorters_memory)
      {
        // Set up input
        arc_stream<> inner_arcs(inner_arcs_file);
        up__arc_stream__decorator decorated_arcs(inner_arcs, outer_arcs);

        level_info_stream<> inner_levels(inner_arcs_file);

        // Set up (decorated) priority queue
        inner_pq_t inner_pq({inner_arcs_file}, inner_pq_memory, inner_pq_max_size);

        using decorator_t = up__pq_decorator<inner_pq_t, outer_pq_t>;
        decorator_t decorated_pq(inner_pq, outer_pq);

        const size_t internal_sorter_can_fit =
          internal_sorter<node>::memory_fits(inner_sorters_memory / 2);

        // Process bottom-up each level
        while (inner_levels.can_pull()) {
          adiar_debug(decorated_arcs.can_pull_terminal() || !decorated_pq.empty(),
                      "If there is a level, then there should also be something for it.");
          const level_info inner_level_info = inner_levels.pull();

          const typename inner_up_sweep::label_t level = inner_level_info.level();
          const size_t level_width = inner_level_info.width();

          adiar_invariant(!decorated_pq.has_current_level() || level == decorated_pq.current_level(),
                          "level and priority queue should be in sync");

          // TODO (optimisation):
          //   use '__reduce_level__fast(...)' iff
          //   - 'NEVER_CANONICAL' (constexpr)
          //   - 'FINAL_CANONICAL' (constexpr)
          //   - 'AUTO' and 'fast_reduce_threshold()' is satisfiable (!constexpr)
          //     NOTE: This probably should be a (possibly) different
          //           threshold than for the outer

          if(level_width <= internal_sorter_can_fit) {
            __reduce_level<inner_up_sweep, internal_sorter>
              (decorated_arcs, level, decorated_pq, outer_writer, inner_sorters_memory, level_width);
          } else {
            __reduce_level<inner_up_sweep, external_sorter>
              (decorated_arcs, level, decorated_pq, outer_writer, inner_sorters_memory, level_width);
          }
        }

        // Forward arcs from outer sweep that collapsed to a sink.
        while (inner_arcs.can_pull_terminal()) {
          const arc a = inner_arcs.pull_terminal();
          adiar_debug(a.source().is_flagged(),
                      "Left-over terminal arcs are meant for outer sweep");
          outer_pq.push(arc(unflag(a.source()), a.target()));
        }
      }

      //////////////////////////////////////////////////////////////////////////
      /// \brief Execute the Inner Up Sweep (part 1).
      ///
      /// \sa nested_sweep
      //////////////////////////////////////////////////////////////////////////
      template<typename inner_up_sweep, typename outer_pq_t>
      void
      up(const arc_stream<> &outer_arcs,
         outer_pq_t &outer_pq,
         node_writer &outer_writer,
         const typename inner_up_sweep::shared_arcs_t &inner_arcs_file,
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

        const size_t inner_pq_bound = inner_arcs_file->max_1level_cut;

        const size_t inner_pq_max_size = memory_mode == memory_mode_t::INTERNAL
          ? std::min(inner_pq_memory_fits, inner_pq_bound)
          : inner_pq_bound;

        const bool external_only = memory_mode == memory_mode_t::EXTERNAL;
        if (!external_only && inner_pq_max_size <= no_lookahead_bound(1)) {
#ifdef ADIAR_STATS
          stats.inner.up.lpq.unbucketed += 1u;
#endif
          using inner_pq_t = typename inner_up_sweep::template pq_t<0, memory_mode_t::INTERNAL>;
          up<inner_up_sweep, inner_pq_t>(outer_arcs, outer_pq, outer_writer,
                                         inner_arcs_file, inner_pq_memory, inner_pq_max_size, inner_sorters_memory);

        } else if(!external_only && inner_pq_max_size <= inner_pq_memory_fits) {
#ifdef ADIAR_STATS
          stats.inner.up.lpq.internal += 1u;
#endif
          using inner_pq_t = typename inner_up_sweep::template pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode_t::INTERNAL>;
          up<inner_up_sweep, inner_pq_t>(outer_arcs, outer_pq, outer_writer,
                                         inner_arcs_file, inner_pq_memory, inner_pq_max_size, inner_sorters_memory);
        } else {
#ifdef ADIAR_STATS
          stats.inner.up.lpq.external += 1u;
#endif
          using inner_pq_t = typename inner_up_sweep::template pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode_t::EXTERNAL>;
          up<inner_up_sweep, inner_pq_t>(outer_arcs, outer_pq, outer_writer,
                                         inner_arcs_file, inner_pq_memory, inner_pq_max_size, inner_sorters_memory);
        }
      }
    } // namespace inner
  } // namespace nested_sweeping

  //////////////////////////////////////////////////////////////////////////////
  // TODO: documentation
  //////////////////////////////////////////////////////////////////////////////
  template<typename outer_up_sweep,
           typename inner_down_sweep,
           size_t outer_look_ahead,
           memory_mode_t outer_mem_mode,
           typename inner_up_sweep>
  typename outer_up_sweep::reduced_t
  __nested_sweep(const typename outer_up_sweep::shared_arcs_t &dag,
                 inner_down_sweep &inner_impl,
                 const size_t outer_pq_memory,
                 const size_t outer_roots_memory,
                 const size_t outer_pq_roots_max,
                 const size_t inner_memory)
  {
    using inner_iter_t = nested_sweeping::outer::inner_iterator<inner_down_sweep>;
    inner_iter_t inner_iter(dag, inner_impl);

    using level_t = typename outer_up_sweep::label_t;
    level_t next_inner = inner_iter.next_inner();

    // If there are no levels to do an inner sweep, then bail out with the
    // classic Reduce sweep.
    if (next_inner == inner_iter_t::NONE) {
#ifdef ADIAR_STATS
      // TODO
#endif
      return reduce<outer_up_sweep>(dag);
    }

    // Set up input
    arc_stream<> outer_arcs(dag);

    // Set up (intermediate) output
    typename outer_up_sweep::shared_nodes_t outer_file
      = __reduce_init_output<outer_up_sweep>();

    node_writer outer_writer(outer_file);

    // Outer Up Sweep: Obtain access to the levels to get the inputs width.
    level_info_stream outer_levels(dag);

    // Outer Up Sweep: Instantiate the (levelized) priority queue and other
    // Reduce state variables, e.g. i-level cuts.
    adiar_debug(outer_pq_memory + outer_roots_memory + inner_memory < memory_available(),
                "Enough memory should be left priority queue and inner sweep");

    using outer_pq_t = typename outer_up_sweep::template pq_t<outer_look_ahead, outer_mem_mode>;
    outer_pq_t outer_pq({dag}, outer_pq_memory, outer_pq_roots_max);

    // Outer Up Sweep: Use the 'inner_memory' for the per-level data structures
    // and streams. This is safe, since these data structures are only
    // instantiated when the Inner Sweep is not.
    const size_t outer_sorters_memory =
      inner_memory - tpie::file_stream<mapping>::memory_usage();

    const size_t outer_internal_sorter_can_fit =
      internal_sorter<node>::memory_fits(outer_sorters_memory / 2);

    // TODO: assert enough memory is still available for the remaining things

    // Inner Down Sweep:
    //
    // 1. We need to create requests for roots of the Inner Down Sweep in (i)
    //    the Outer Down Sweep and (ii) the Inner Up Sweep.
    //
    // 2. We want to change the memory type based on i-level cuts if possible.
    //    To keep this at the compile-time level, we cannot instantiate the
    //    priority queue at this level of the algorithm.
    //
    // Hence, we will have a special sorter of all nested root requests which is
    // then merged with the recursion requests within the Inner Down Sweep.
    //
    // The arcs in 'outer_roots' are always between nodes that cross some
    // level of the input. That is, these arcs always constitute a 1-level cut
    // of the input.
    using outer_roots_t =
      nested_sweeping::outer::roots_sorter<outer_mem_mode,
                                           typename inner_down_sweep::request_t,
                                           typename inner_down_sweep::request_pred_t>;
    outer_roots_t outer_roots(outer_roots_memory, outer_pq_roots_max);

    // Decorator that acts as 'outer_pq' but moves arcs to 'outer_roots'.
    using outer_pq_decorator_t =
      nested_sweeping::outer::up__pq_decorator<outer_pq_t, outer_roots_t>;

    while (outer_levels.can_pull()) {
      adiar_debug(outer_arcs.can_pull_terminal() || !outer_pq.empty(),
                  "If there is a level, then there should also be something for it.");

      // Set up next level for outer reduce

      const level_info outer_level = outer_levels.pull();

      adiar_invariant(!outer_pq.has_current_level()
                      || outer_level.level() == outer_pq.current_level(),
                      "level and priority queue should be in sync");
      adiar_invariant(next_inner == inner_iter_t::NONE || next_inner <= outer_level.level(),
                      "next_inner level should (if it exists) be above current level (inclusive).");

      // -----------------------------------------------------------------------
      // CASE Unnested Level with no nested sweep above:
      //   Reduce this level (as usual).
      if (next_inner == inner_iter_t::NONE) {
        // TODO (optimisation):
        //   use '__reduce_level__fast(...)' iff 'NEVER_CANONICAL' (constexpr).

        if(outer_level.width() <= outer_internal_sorter_can_fit) {
          __reduce_level<outer_up_sweep, internal_sorter>
            (outer_arcs, outer_level.level(), outer_pq, outer_writer,
             outer_sorters_memory, outer_level.width());
        } else {
          __reduce_level<outer_up_sweep, external_sorter>
            (outer_arcs, outer_level.level(), outer_pq, outer_writer,
             outer_sorters_memory, outer_level.width());
        }

        continue;
      }

      // -----------------------------------------------------------------------
      // CASE: Unnested Level with a nested sweep above:
      //   Reduce this level (but with a decorator to redirect requests).
      if (next_inner < outer_level.level()) {
        outer_pq_decorator_t outer_pq_decorator(outer_pq, outer_roots, next_inner);

        // TODO (optimisation):
        //   use '__reduce_level__fast(...)' iff
        //   - 'NEVER_CANONICAL' (constexpr)
        //   - 'FINAL_CANONICAL' (constexpr)
        //   - 'AUTO' and 'fast_reduce_threshold()' is satisfiable (!constexpr)
        //     NOTE: This probably should be a (possibly) different threshold
        //           than for the inner

        if(outer_level.width() <= outer_internal_sorter_can_fit) {
          __reduce_level<outer_up_sweep, internal_sorter>
            (outer_arcs, outer_level.level(), outer_pq_decorator, outer_writer,
             outer_sorters_memory, outer_level.width());
        } else {
          __reduce_level<outer_up_sweep, external_sorter>
            (outer_arcs, outer_level.level(), outer_pq_decorator, outer_writer,
             outer_sorters_memory, outer_level.width());
        }

        continue;
      }

      // -----------------------------------------------------------------------
      // CASE Nested Level:
      //   Sweep down re-reduce it back up to this level.
      adiar_debug(outer_level.level() == next_inner,
                  "'next_inner' level is not skipped");

      // -----------------------------------------------------------------------
      // Collect all recursions for this level

      // Keep track of whether a non-GC request has been made.
      bool non_gc_request = false;
      {
        outer_pq_decorator_t outer_pq_decorator(outer_pq, outer_roots, next_inner);

        // TODO (bdd_compose):
        //   Support creating nodes on this level.

        while ((outer_arcs.can_pull_terminal()
                && outer_arcs.peek_terminal().source().label() == outer_level.level())
               || outer_pq.can_pull()) {

          const arc e_high = __reduce_get_next(outer_pq, outer_arcs);
          const arc e_low  = __reduce_get_next(outer_pq, outer_arcs);

          const node n = node_of(e_low, e_high);

          // Apply Reduction rule 1?
          const node::ptr_t reduction_rule_ret = outer_up_sweep::reduction_rule(n);
          if (reduction_rule_ret != n.uid()) {
            // If so, preserve child in inner sweep
            if (!outer_levels.can_pull()) {
              adiar_debug(!outer_arcs.can_pull_internal(), "Should not have any parents at top-most level");

              if (reduction_rule_ret.is_terminal()) {
                return typename outer_up_sweep::reduced_t(reduction_rule_ret.value());
              }
              outer_pq_decorator.push(arc(node::ptr_t::NIL(), flag(reduction_rule_ret)));
            } else {
              do {
                outer_pq_decorator.push(arc(outer_arcs.pull_internal().source(), flag(reduction_rule_ret)));
              } while (outer_arcs.can_pull_internal() && outer_arcs.peek_internal().target() == n.uid());
            }
          } else {
            // Otherwise, create request
            if (!outer_levels.can_pull()) {
              adiar_debug(!outer_arcs.can_pull_internal(), "Should not have any parents at top-most level");

              const typename inner_down_sweep::request_t r =
                inner_impl.request_from_node(n, node::ptr_t::NIL());

              adiar_debug(r.targets() > 0, "Requests are always to something");
              non_gc_request |= r.targets() > 1;

              if (r.target.fst().is_terminal()) {
                return typename outer_up_sweep::reduced_t(r.target.fst().value());
              }
              outer_pq_decorator.push(r);
            } else {
              do {
                const typename inner_down_sweep::request_t r =
                  inner_impl.request_from_node(n, outer_arcs.pull_internal().source());

                adiar_debug(r.targets() > 0, "Requests are always to something");
                non_gc_request |= r.targets() > 1;

                outer_pq_decorator.push(r);
              } while (outer_arcs.can_pull_internal() && outer_arcs.peek_internal().target() == n.uid());
            }
          }
        }
      }

      // -----------------------------------------------------------------------
      // Obtain the next level to sweep nestedly on.
      next_inner = inner_iter.next_inner();

      const bool is_last_inner = next_inner == inner_iter_t::NONE;

      const bool run_inner =
        // We have some requests to sweep on
        outer_roots.size() > 0
        // There are some requests that change the subgraph, or this is the last
        // down sweep (meaning we have to go down to ensure dead subtrees are
        // garbage collected and nodes are canonical)
        && (non_gc_request || is_last_inner);

      if (run_inner) {
        // ---------------------------------------------------------------------
        // Inner Down Sweep
#ifdef ADIAR_STATS
        // TODO
#endif
        adiar_debug(outer_roots.size() > 0,
                    "Nested Sweep needs some number of requests");

        outer_writer.detach();

        // TODO (optimisation): is_last_inner && !non_gc_request
        //   Use a simpler (and hence faster?) algorithm for a GC-only sweep.

        const typename inner_down_sweep::unreduced_t inner_unreduced =
          nested_sweeping::inner::down(inner_impl, outer_file, outer_roots, inner_memory);

        if (inner_unreduced.template has<typename outer_up_sweep::shared_nodes_t>()) {
          adiar_debug(!outer_levels.can_pull(),
                      "Should only collapse to a node file case when at the very top-level.");
          adiar_debug(inner_unreduced.template get<typename outer_up_sweep::shared_nodes_t>()->is_terminal(),
                      "Should have collapsed to a terminal.");

          return inner_unreduced.template get<typename outer_up_sweep::shared_nodes_t>();
        }

        // ---------------------------------------------------------------------
        // Inner Up Sweep
        adiar_debug(!inner_unreduced.template has<no_file>(),
                    "Inner Sweep returned something");

        const typename outer_up_sweep::shared_arcs_t inner_arcs =
          inner_unreduced.template get<typename outer_up_sweep::shared_arcs_t>();

        outer_file = __reduce_init_output<outer_up_sweep>();
        outer_writer.attach(outer_file);

        if (is_last_inner) {
          nested_sweeping::inner::up<inner_up_sweep>(outer_arcs, outer_pq, outer_writer,
                                                     inner_arcs, inner_memory);
        } else {
          adiar_debug(next_inner < outer_level.level(),
                      "If 'next_inner' is not illegal, then it is above current level");

          outer_pq_decorator_t outer_pq_decorator(outer_pq, outer_roots, next_inner);

          nested_sweeping::inner::up<inner_up_sweep>(outer_arcs, outer_pq_decorator, outer_writer,
                                                     inner_arcs, inner_memory);
        }
      } else if (outer_roots.size() > 0) {
        // ---------------------------------------------------------------------
        // Bail out requests for a GC-only Inner Sweep (postponing doing so for
        // a later sweep)
#ifdef ADIAR_STATS
        // TODO
#endif
        adiar_debug(next_inner <= outer_up_sweep::ptr_t::MAX_LABEL,
                    "Has another later sweep to do possible garbage collection");

        adiar_debug(outer_roots_memory <= inner_memory,
                    "Enough memory is left for a temporary root sorter");

        outer_roots_t tmp_outer_roots(outer_roots_memory, outer_pq_roots_max);
        outer_pq_decorator_t outer_pq_decorator(outer_pq, tmp_outer_roots, next_inner);

        outer_roots.sort();
        while (outer_roots.can_pull()) {
          const typename inner_down_sweep::request_t r = outer_roots.pull();
          adiar_debug(r.targets() == 1, "Has exactly one child");

          outer_pq_decorator.push(arc(unflag(r.data.source), r.target.fst()));
        }

        outer_roots.move(tmp_outer_roots);
      } else { // if (outer_roots.size() == 0) {
        // ---------------------------------------------------------------------
        // Nothing within 'outer_file' should survive
#ifdef ADIAR_STATS
        // TODO
#endif
        if (outer_writer.size() != 0) {
          outer_writer.detach();
          outer_file = __reduce_init_output<outer_up_sweep>();
          outer_writer.attach(outer_file);
        }
      }

      // -----------------------------------------------------------------------
      // Set up next level in Outer PQ
      if (!outer_pq.empty() || !outer_roots.empty()) {
        adiar_debug(!outer_arcs.can_pull_terminal() || outer_arcs.peek_terminal().source().label() < outer_level.level(),
                    "All terminal arcs for 'label' should be processed");

        adiar_debug(!outer_arcs.can_pull_internal() || outer_arcs.peek_internal().target().label() < outer_level.level(),
                    "All internal arcs for 'label' should be processed");

        adiar_debug(outer_pq.empty() || !outer_pq.can_pull(),
                    "All forwarded arcs for 'label' should be processed");

        const size_t terminal_stop_level =
          outer_arcs.can_pull_terminal() ? outer_arcs.peek_terminal().source().label() : outer_pq_t::NO_LABEL;

        const size_t outer_roots_stop_level =
          !outer_roots.empty() ? outer_roots.deepest_source() : outer_pq_t::NO_LABEL;

        adiar_debug(terminal_stop_level != outer_pq_t::NO_LABEL
                    || outer_roots_stop_level != outer_pq_t::NO_LABEL
                    || !outer_pq.empty(),
                    "There must be some (known) level ready to be forwarded to.");

        const size_t stop_level = terminal_stop_level == outer_pq_t::NO_LABEL    ? outer_roots_stop_level
                                : outer_roots_stop_level == outer_pq_t::NO_LABEL ? terminal_stop_level
                                : std::max(terminal_stop_level, outer_roots_stop_level)
          ;

        adiar_debug(stop_level != outer_pq_t::NO_LABEL || !outer_pq.empty(),
                    "There must be some (known) level ready to be forwarded to.");

        outer_pq.setup_next_level(stop_level);
      } else if (outer_file->is_terminal()) {
#ifdef ADIAR_STATS
        // TODO
#endif
        return outer_file;
      }
    }

    // Return (now not anymore 'intermediate') output
    outer_writer.detach();
    return outer_file;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Framework for I/O-efficient translation of recursive functions that
  ///        re-recurse on intermediate results.
  ///
  /// \tparam outer_up_sweep   Policy for outer bottom-up (Reduce) sweep.
  ///
  /// \tparam inner_down_sweep Policy for the inner downwards sweep that
  ///                          manipulates the graph.
  ///
  /// \tparam inner_up_sweep   Policy for inner bottom-up (Reduce) sweep.
  ///
  /// \param input             A possibly reduced input. If it has collapsed to a
  ///                          terminal, then that one is returned. If it is
  ///                          reduced, then it will be transposed before
  ///                          computation starts.
  ///
  /// \param  inner_impl Provides the non-static parts of the inner logic that
  ///         determines when to start the nested sweep.
  //////////////////////////////////////////////////////////////////////////////
  template<typename outer_up_sweep,
           typename inner_down_sweep,
           typename inner_up_sweep = outer_up_sweep>
  typename outer_up_sweep::reduced_t
  nested_sweep(const typename outer_up_sweep::unreduced_t &input,
               inner_down_sweep &inner_impl)
  {
    adiar_debug(!input.empty(), "Input for Nested Sweeping should always be non-empty");

    using shared_arcs_t  = typename outer_up_sweep::shared_arcs_t;
    using shared_nodes_t = typename outer_up_sweep::shared_nodes_t;

    // Is it a terminal?
    if (input.template has<shared_nodes_t>() && input.template get<shared_nodes_t>()->is_terminal()) {
      return typename outer_up_sweep::reduced_t(input.template get<shared_nodes_t>(),
                                                input.negate);
    }

    // Otherwise obtain the semi-transposed DAG (construct it if necessary)
    const typename outer_up_sweep::shared_arcs_t dag = input.template has<shared_arcs_t>()
      ? input.template get<shared_arcs_t>()
      : transpose(typename outer_up_sweep::reduced_t(input.template get<shared_nodes_t>(), input.negate));

    // Compute amount of memory available for auxiliary data structures after
    // having opened all streams.
    //
    // We then may derive an upper bound on the size of auxiliary data
    // structures for the Outer Up Sweep. If small enough, we can run them with
    // a faster internal memory variant.
    //
    // Here, we still need to keep in mind, that the available memory needs to
    // be distributed across (up to) four priority queues!
    const size_t total_memory = memory_available();

    const size_t outer_memory = total_memory / 2;
    const size_t inner_memory = total_memory - outer_memory;

    const size_t aux_outer_memory = outer_memory
      // Input streams
      - arc_stream<>::memory_usage()
      - level_info_stream<>::memory_usage()
      // Inner Iterator
      - nested_sweeping::outer::inner_iterator<inner_down_sweep>::memory_usage()
      // Output streams
      - node_writer::memory_usage();

    constexpr size_t data_structures_in_pq =
      outer_up_sweep::template pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode_t::INTERNAL>::DATA_STRUCTURES;


    using internal_roots_sorter_t =
      nested_sweeping::outer::roots_sorter<memory_mode_t::INTERNAL,
                                           typename inner_down_sweep::request_t,
                                           typename inner_down_sweep::request_pred_t>;

    constexpr size_t data_structures_in_roots = internal_roots_sorter_t::DATA_STRUCTURES;

    const size_t outer_pq_memory =
      (aux_outer_memory / (data_structures_in_pq + data_structures_in_roots)) * data_structures_in_pq;

    const size_t outer_roots_memory = aux_outer_memory - outer_pq_memory;

    const tpie::memory_size_type outer_pq_memory_fits =
      outer_up_sweep::template pq_t<ADIAR_LPQ_LOOKAHEAD, memory_mode_t::INTERNAL>::memory_fits(outer_pq_memory);

    const tpie::memory_size_type outer_roots_memory_fits =
      internal_roots_sorter_t::memory_fits(outer_pq_memory);

    const size_t pq_roots_bound = dag->max_1level_cut;

    const size_t outer_pq_roots_max = memory_mode == memory_mode_t::INTERNAL
      ? std::min({outer_pq_memory_fits, outer_roots_memory_fits, pq_roots_bound})
      : pq_roots_bound;

    const bool external_only = memory_mode == memory_mode_t::EXTERNAL;
    if (!external_only && outer_pq_roots_max <= no_lookahead_bound(1)) {
#ifdef ADIAR_STATS
      stats_reduce.lpq.unbucketed += 1u;
#endif
      return __nested_sweep<outer_up_sweep, inner_down_sweep, 0, memory_mode_t::INTERNAL, inner_up_sweep>
        (dag, inner_impl, outer_pq_memory, outer_roots_memory, outer_pq_roots_max, inner_memory);
    } else if(!external_only && outer_pq_roots_max <= outer_pq_memory_fits && outer_pq_roots_max <= outer_roots_memory_fits) {
#ifdef ADIAR_STATS
      stats_reduce.lpq.internal += 1u;
#endif
      return __nested_sweep<outer_up_sweep, inner_down_sweep, ADIAR_LPQ_LOOKAHEAD, memory_mode_t::INTERNAL, inner_up_sweep>
        (dag, inner_impl, outer_pq_memory, outer_roots_memory, outer_pq_roots_max, inner_memory);
    } else {
#ifdef ADIAR_STATS
      stats_reduce.lpq.external += 1u;
#endif
      return __nested_sweep<outer_up_sweep, inner_down_sweep, ADIAR_LPQ_LOOKAHEAD, memory_mode_t::EXTERNAL, inner_up_sweep>
        (dag, inner_impl, outer_pq_memory, outer_roots_memory, outer_pq_roots_max, inner_memory);
    }
  }
}

#endif // ADIAR_INTERNAL_ALGORITHMS_NESTED_SWEEPING_H

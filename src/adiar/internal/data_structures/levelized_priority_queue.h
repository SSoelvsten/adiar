#ifndef ADIAR_INTERNAL_DATA_STRUCTURES_LEVELIZED_PRIORITY_QUEUE_H
#define ADIAR_INTERNAL_DATA_STRUCTURES_LEVELIZED_PRIORITY_QUEUE_H

#include <limits>

#include <tpie/tpie.h>

#include <tpie/file.h>
#include <tpie/file_stream.h>
#include <tpie/sort.h>

#include <adiar/memory_mode.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/util.h>
#include <adiar/internal/data_structures/priority_queue.h>
#include <adiar/internal/data_structures/sorter.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/statistics.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Defines at compile time the type of the file stream to use for
  ///        reading the levels from some file(s).
  //////////////////////////////////////////////////////////////////////////////
  template <typename file_t>
  struct label_stream_t
  {
    typedef level_info_stream<typename file_t::elem_t> stream_t;
  };

  template <>
  struct label_stream_t<label_file>
  {
    typedef label_stream<> stream_t;
  };

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Number of items for which the unbucketed queue can be used
  ////////////////////////////////////////////////////////////////////////////
  constexpr size_t no_lookahead_bound(const size_t degree = 1)
  {
    return 8 * (1 << degree);
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Strictly less ' < ' between two levels.
  ////////////////////////////////////////////////////////////////////////////
  template<typename level_comp_t>
  inline bool level_cmp_lt(const ptr_uint64::label_t l1,
                           const ptr_uint64::label_t l2,
                           const level_comp_t &level_comp)
  {
    return level_comp(l1, l2);
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Less or equal ' <= ' between two levels.
  ////////////////////////////////////////////////////////////////////////////
  template<typename level_comp_t>
  inline bool level_cmp_le(const ptr_uint64::label_t l1,
                           const ptr_uint64::label_t l2,
                           const level_comp_t &level_comp)
  {
    return level_comp(l1, l2) || l1 == l2;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Merges the labels from one or more files.
  ///
  /// \param file_t Type of the files to read from
  ///
  /// \param comp_t Comparator with which to merge the labels
  ///
  /// \param FILES  Number of files to read from
  //////////////////////////////////////////////////////////////////////////////
  template <typename file_t, typename comp_t, size_t FILES>
  class label_merger
  {
    static_assert(0 < FILES,
                  "At least one file should be merged");

    typedef typename label_stream_t<file_t>::stream_t stream_t;

  public:
    static size_t memory_usage()
    {
      return FILES * stream_t::memory_usage();
    }

  private:
    comp_t _comparator = comp_t();

    unique_ptr<stream_t> _label_streams [FILES];

  public:
    void hook(const file_t (&fs) [FILES])
    {
      for (size_t idx = 0u; idx < FILES; idx++) {
        _label_streams[idx] = make_unique<stream_t>(fs[idx]);
      }
    }

    void hook(const dd (&dds) [FILES])
    {
      for (size_t idx = 0u; idx < FILES; idx++) {
        _label_streams[idx] = make_unique<stream_t>(dds[idx].file);
      }
    }

    bool can_pull()
    {
      for (size_t idx = 0u; idx < FILES; idx++) {
        if (_label_streams[idx] -> can_pull()) {
          return true;
        }
      }
      return false;
    }

    ptr_uint64::label_t peek()
    {
      adiar_debug(can_pull(),
                  "Cannot peek past end of all streams");

      bool has_min_label = false;
      ptr_uint64::label_t min_label = 0u;
      for (size_t idx = 0u; idx < FILES; idx++) {
        if (_label_streams[idx] -> can_pull()
            && (!has_min_label || _comparator(__label_of<>(_label_streams[idx] -> peek()), min_label))) {
          has_min_label = true;
          min_label = __label_of<>(_label_streams[idx] -> peek());
        }
      }

      return min_label;
    }

    ptr_uint64::label_t pull()
    {
      adiar_debug(can_pull(),
                  "Cannot pull past end of all streams");

      ptr_uint64::label_t min_label = peek();

      // pull from all with min_label
      for (const unique_ptr<stream_t> &level_info_stream : _label_streams) {
        if (level_info_stream -> can_pull() && __label_of<>(level_info_stream -> peek()) == min_label) {
          level_info_stream -> pull();
        }
      }

      return min_label;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// The preprocessor variable ADIAR_PQ_LOOKAHEAD can be used to change the
  /// number of buckets used by the levelized priority queue.
  //////////////////////////////////////////////////////////////////////////////
#ifndef ADIAR_LPQ_LOOKAHEAD
#define ADIAR_LPQ_LOOKAHEAD 1u
#endif

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Struct holding statistics on the levelized priority queue
  //////////////////////////////////////////////////////////////////////////////
  extern stats_t::levelized_priority_queue_t stats_levelized_priority_queue;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Levelized Priority Queue with a finite circular array of sorters
  ///          and an overflow priority queue.
  ///
  /// \details Improves performance of a regular priority queue by exploiting
  ///          the performance difference between sorting a list and populating
  ///          a priority queue. This improving performance by placing all
  ///          pushed queue elements into a bucket for its specific level and
  ///          then sorting it when one finally arrives at said level. If no
  ///          bucket exists for said element, then this overflow will be placed
  ///          in an priority queue to then be later be merged with its bucket.
  ///
  /// \remark  Elements may \e only be pushed to a \e later level than the
  ///          currently read level.
  ///
  /// \param elem_t       Type of items to be placed in the priority queue
  ///
  /// \param elem_level_t Provides statically a function named 'label_of' to
  ///                     retrieve in which bucket to place the given element
  ///
  /// \param elem_comp_t  Sorting comparator to use in the inner queue buckets
  ///                     and for merging
  ///
  /// \param file_t       Type of the files to obtain the relevant levels from
  ///
  /// \param FILES        Number of files to obtain the levels from
  ///
  ///
  /// \param level_comp_t Comparator to be used for merging multiple levels from
  ///                     the files together (std::less = top-down, while
  ///                     std::greater = bottom-up)
  ///
  /// \param INIT_LEVEL   The index for the first level one can push to. In other
  ///                     words, the number of levels to 'skip'.
  ///
  /// \param LOOK_AHEAD   The number of levels (ahead of the current)
  ///                     explicitly handle with a sorting algorithm
  //////////////////////////////////////////////////////////////////////////////

  template <typename elem_t,
            typename elem_level_t,
            typename elem_comp_t         = std::less<elem_t>,
            ptr_uint64::label_t  LOOK_AHEAD          = ADIAR_LPQ_LOOKAHEAD,
            memory_mode_t mem_mode = memory_mode_t::EXTERNAL,
            typename file_t              = levelized_file<elem_t>,
            size_t   FILES               = 1u,
            typename level_comp_t        = std::less<ptr_uint64::label_t>,
            ptr_uint64::label_t  INIT_LEVEL          = 1u
            >
  class levelized_priority_queue
  {
  private:
    static_assert(0 < LOOK_AHEAD,
                  "LOOK_AHEAD must at least be of one level");

    // TODO: LOOK_AHEAD must be strictly smaller than MAX_LABEL (but we need to
    //       close #164 first to do this check at compile time)

    static constexpr ptr_uint64::label_t OUT_OF_BUCKETS_IDX = static_cast<ptr_uint64::label_t>(-1);

    static_assert(LOOK_AHEAD + 1 < OUT_OF_BUCKETS_IDX,
                  "LOOK_AHEAD must not be so large to also include '-1'");

    static_assert(OUT_OF_BUCKETS_IDX + 1 == 0,
                  "Overflow to '0' is necessary for internal logic to work");

    static_assert(ptr_uint64::MAX_LABEL+1 > ptr_uint64::MAX_LABEL,
                  "'ptr_uint64::label_t' should leave a window of at least one above 'MAX_LABEL'");

    static constexpr ptr_uint64::label_t NO_LABEL = ptr_uint64::MAX_LABEL+1;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the sorter for each bucket.
    ////////////////////////////////////////////////////////////////////////////
    typedef sorter<mem_mode, elem_t, elem_comp_t> sorter_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the overflow priority queue.
    ////////////////////////////////////////////////////////////////////////////
    typedef priority_queue<mem_mode, elem_t, elem_comp_t> priority_queue_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of buckets.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr size_t BUCKETS = LOOK_AHEAD + 1;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Total number of data structures in Levelized Priority Queue.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr size_t DATA_STRUCTURES =
      BUCKETS * sorter_t::DATA_STRUCTURES + priority_queue_t::DATA_STRUCTURES;

  private:
    static tpie::memory_size_type const_memory_usage()
    {
      return label_merger<file_t, level_comp_t, FILES>::memory_usage();
    }

  public:
    static tpie::memory_size_type memory_usage(tpie::memory_size_type no_elements)
    {
      return priority_queue<memory_mode_t::INTERNAL, elem_t, elem_comp_t>::memory_usage(no_elements)
        + BUCKETS * sorter<memory_mode_t::INTERNAL, elem_t, elem_comp_t>::memory_usage(no_elements)
        + const_memory_usage();
    }

    static tpie::memory_size_type memory_fits(tpie::memory_size_type memory_bytes)
    {
      const size_t const_memory_bytes = const_memory_usage();

      if (memory_bytes < const_memory_bytes) {
        return 0u;
      }

      // HACK: just provide the minimum of what either of the data structures
      //       can hold when given an equal share of the memory.
      const size_t memory_per_data_structure = (memory_bytes - const_memory_bytes) / DATA_STRUCTURES;

      const size_t sorter_fits = sorter<memory_mode_t::INTERNAL, elem_t, elem_comp_t>
        ::memory_fits(memory_per_data_structure);

      const size_t priority_queue_fits = priority_queue<memory_mode_t::INTERNAL, elem_t, elem_comp_t>
        ::memory_fits(memory_per_data_structure);

      return std::min(sorter_fits, priority_queue_fits);
    }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of elements in the levelized priority queue (excluding the
    ///        one element possibly in '_top_elem').
    ////////////////////////////////////////////////////////////////////////////
    size_t _size = 0;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Cache for the top element.
    ///
    /// \sa      levelized_priority_queue::_has_top_elem
    ///
    /// \details Since the priority queue is used in a levelized fashion, then
    ///          the current level is completely frozen. Hence, the top element
    ///          cannot change on a push.
    ////////////////////////////////////////////////////////////////////////////
    elem_t _top_elem;
    bool _has_top_elem = false;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Instantiation of the comparator between levels.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_t _current_level = NO_LABEL;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Instantiation of the comparator between levels.
    ////////////////////////////////////////////////////////////////////////////
    level_comp_t _level_comparator = level_comp_t();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Instantiation of the comparator between elements.
    ////////////////////////////////////////////////////////////////////////////
    elem_comp_t _e_comparator = elem_comp_t();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Maximum size of levelized priority queue.
    ////////////////////////////////////////////////////////////////////////////
    const size_t _max_size;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Total memory given in constructor.
    ////////////////////////////////////////////////////////////////////////////
    const tpie::memory_size_type _memory_given;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Memory used by the label merger.
    ////////////////////////////////////////////////////////////////////////////
    const tpie::memory_size_type _memory_occupied_by_merger =
      label_merger<file_t, level_comp_t, FILES>::memory_usage();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Memory to be used for the buckets.
    ////////////////////////////////////////////////////////////////////////////
    const tpie::memory_size_type _memory_for_buckets;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Memory to be used for the overflow priority queue.
    ////////////////////////////////////////////////////////////////////////////
    const tpie::memory_size_type _memory_occupied_by_overflow;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Provides the levels to distribute all elements across.
    ////////////////////////////////////////////////////////////////////////////
    label_merger<file_t, level_comp_t, FILES> _level_merger;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Level of each bucket.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_t _buckets_level [BUCKETS];

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Sorter for each bucket.
    ////////////////////////////////////////////////////////////////////////////
    unique_ptr<sorter_t> _buckets_sorter [BUCKETS];

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Index of the currently read-from bucket (if any).
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_t _front_bucket_idx = OUT_OF_BUCKETS_IDX;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Index of the last available bucket (if any).
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_t _back_bucket_idx  = OUT_OF_BUCKETS_IDX;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Next element to take out of the bucket (if any)
    ///
    /// \sa    levelized_priority_queue::_has_next_from_bucket
    ///
    /// \todo  Currently the implementation of the sorter does not allow one to
    ///        peek the next value. We can decrease code complexity by moving
    ///        this into the sorter's logic.
    ////////////////////////////////////////////////////////////////////////////
    elem_t _next_from_bucket;
    bool _has_next_from_bucket = false;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Overflow priority queue used for elements pushed to a level where
    ///        a bucket is (yet) not created.
    ////////////////////////////////////////////////////////////////////////////
    priority_queue_t _overflow_queue;

#ifdef ADIAR_STATS
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The actual maximum size of the levelized priority queue.
    ////////////////////////////////////////////////////////////////////////////
    size_t _actual_max_size = 0u;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Reference to struct to store non-global stats into.
    ////////////////////////////////////////////////////////////////////////////
    stats_t::levelized_priority_queue_t &_stats;
#endif

  private:
    static tpie::memory_size_type mem_overflow_queue(tpie::memory_size_type memory_given)
    {
      const size_t const_memory = const_memory_usage();

      adiar_debug(const_memory < memory_given,
                  "There should be enough memory for the merger");

      // subtract memory of the merger to not take any of its memory.
      memory_given -= const_memory;

      // GCC bug 85282: One cannot do a member class specialization of each of
      // the two following cases. So, we will have to resort to a constexpr
      // if-statement instead.
      if constexpr (mem_mode == memory_mode_t::INTERNAL) {
        // ---------------------------------------------------------------------
        // INTERNAL MEMORY MODE:
        //   Divide memory in equal parts

        return memory_given / DATA_STRUCTURES;
      } else if constexpr (mem_mode == memory_mode_t::EXTERNAL) {
        // ---------------------------------------------------------------------
        // EXTERNAL MEMORY MODE:
        //   Use 1/(4Buckets + 1)th of the memory and at least 8 MiB.

        const tpie::memory_size_type eight_MiB = 8 * 1024;
        const tpie::memory_size_type weighted_share = memory_given / (4 * BUCKETS + 1);

        return std::max(eight_MiB, weighted_share);
      } else {
        // ---------------------------------------------------------------------
        static_assert(mem_mode == memory_mode_t::INTERNAL && mem_mode == memory_mode_t::EXTERNAL,
                      "Memory mode must be 'INTERNAL' or 'EXTERNAL' at compile-time");
      }
    }

    levelized_priority_queue(tpie::memory_size_type memory_given, size_t max_size,
                             [[maybe_unused]] stats_t::levelized_priority_queue_t &stats)
      : _max_size(max_size)
      , _memory_given(memory_given)
      , _memory_for_buckets(memory_given - _memory_occupied_by_merger - mem_overflow_queue(memory_given))
      , _memory_occupied_by_overflow(mem_overflow_queue(memory_given))
      , _overflow_queue(mem_overflow_queue(memory_given), max_size)
#ifdef ADIAR_STATS
      , _stats(stats)
#endif
    {
      adiar_debug(_memory_occupied_by_merger + _memory_for_buckets + _memory_occupied_by_overflow <= _memory_given,
                  "the amount of memory used should be within the given bounds");
    }


  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief              Instantiate with the given amount of memory.
    ///
    /// \param files        Files to follow the levels of
    ///
    /// \param memory_given Total amount of memory to use
    ////////////////////////////////////////////////////////////////////////////
    levelized_priority_queue(const file_t (& files) [FILES],
                             tpie::memory_size_type memory_given,
                             size_t max_size,
                             stats_t::levelized_priority_queue_t &stats)
      : levelized_priority_queue(memory_given, max_size, stats)
    {
      _level_merger.hook(files);
      init_buckets();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief              Instantiate with the given amount of memory.
    ///
    /// \param dds          Decision Diagrams to follow the levels of
    ///
    /// \param memory_given Total amount of memory to use
    ////////////////////////////////////////////////////////////////////////////
    levelized_priority_queue(const dd (& dds) [FILES],
                             tpie::memory_size_type memory_given,
                             size_t max_size,
                             stats_t::levelized_priority_queue_t &stats)
      : levelized_priority_queue(memory_given, max_size, stats)
    {
      _level_merger.hook(dds);
      init_buckets();
    }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Computes final memory usage of internal data structures to then
    ///         distribute the remaining memory when initialising the buckets.
    ///
    /// \remark Call this function at the end of the constructor after the
    ///         level_merger has hooked into the input.
    ////////////////////////////////////////////////////////////////////////////
    void init_buckets()
    {
      // Initially skip the number of levels
      for (ptr_uint64::label_t idx = 0; _level_merger.can_pull() && idx < INIT_LEVEL; idx++) {
        _level_merger.pull();
      }

      // Set up buckets until no levels are left or all buckets have been
      // instantiated. Notice, that _back_bucket_idx was initialised to -1.
      while (_back_bucket_idx + 1 < BUCKETS && _level_merger.can_pull()) {
        const ptr_uint64::label_t level = _level_merger.pull();

        adiar_invariant(_front_bucket_idx == OUT_OF_BUCKETS_IDX,
                        "Front bucket not moved");

        _back_bucket_idx++;

        _buckets_level[_back_bucket_idx] = level;
        _buckets_sorter[_back_bucket_idx] = sorter_t::make_unique(_memory_for_buckets, _max_size, BUCKETS);
      }
    }

  public:
    ~levelized_priority_queue()
    {
#ifdef ADIAR_STATS
      stats_levelized_priority_queue.sum_predicted_max_size += _max_size;
      _stats.sum_predicted_max_size += _max_size;

      stats_levelized_priority_queue.sum_actual_max_size += _actual_max_size;
      _stats.sum_actual_max_size += _actual_max_size;

      stats_levelized_priority_queue.sum_max_size_ratio += frac(_actual_max_size, _max_size);
      _stats.sum_max_size_ratio += frac(_actual_max_size, _max_size);

      stats_levelized_priority_queue.sum_destructors++;
      _stats.sum_destructors++;
#endif
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there is any current level to pull elements from.
    ////////////////////////////////////////////////////////////////////////////
    bool has_current_level() const
    {
      return _current_level != NO_LABEL;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The label of the current level.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_t current_level()
    {
      adiar_debug(has_current_level(),
                  "Needs to have a 'current' level to read the level from");

      return _current_level;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there are any more (possibly all empty) levels.
    ////////////////////////////////////////////////////////////////////////////
    bool has_next_level() const
    {
      return has_next_bucket();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The label of the next (possibly empty) level.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_t next_level() const
    {
      return next_bucket_level();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether one can push elements.
    ////////////////////////////////////////////////////////////////////////////
    bool can_push() const
    {
      return has_next_level();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Push an element into the priority queue.
    ////////////////////////////////////////////////////////////////////////////
    void push(const elem_t &e)
    {
      adiar_debug(can_push(),
                  "Should only push when there is a yet unvisited level.");

      const ptr_uint64::label_t level = elem_level_t::label_of(e);

      adiar_debug(level_cmp_le<level_comp_t>(next_bucket_level(), level, _level_comparator),
                  "Can only push element to next bucket or later.");

      const size_t pushable_buckets = active_buckets() - has_front_bucket();

      adiar_debug(pushable_buckets > 0,
                  "There is at least one pushable bucket (i.e. level)");

      _size++;
#ifdef ADIAR_STATS
      _actual_max_size = std::max(_actual_max_size, _size);
#endif

      ptr_uint64::label_t bucket_offset = 1u;
      do {
        const ptr_uint64::label_t bucket_idx = (_front_bucket_idx + bucket_offset++) % BUCKETS;

        if (_buckets_level[bucket_idx] == level) {
          _buckets_sorter[bucket_idx] -> push(e);
#ifdef ADIAR_STATS
          stats_levelized_priority_queue.push_bucket++;
          _stats.push_bucket++;
#endif
          return;
        }
      } while (bucket_offset <= pushable_buckets);

#ifdef ADIAR_STATS
      stats_levelized_priority_queue.push_overflow++;
      _stats.push_overflow++;
#endif
      _overflow_queue.push(e);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief            Set up the next nonempty level to which some requests
    ///                   have been pushed before the given stop_level.
    ///
    /// \param stop_level The level that should be furthest forwarded to. If no
    ///                   stop level is given then the next level is based on
    ///                   the next existing element in the priority queue.
    ////////////////////////////////////////////////////////////////////////////
    void setup_next_level(ptr_uint64::label_t stop_level = NO_LABEL)
    {
      adiar_debug(stop_level <= ptr_uint64::MAX_LABEL || stop_level == NO_LABEL,
                  "The stop level should be a legal value (or not given)");

      adiar_debug(!has_current_level() || empty_level(),
                  "Level is empty before moving on to the next");

      adiar_debug(stop_level != NO_LABEL || !empty(),
                  "Either a stop level is given or we have some non-empty level to forward to");

      const ptr_uint64::label_t overflow_level = !_overflow_queue.empty()
        ? elem_level_t::label_of(_overflow_queue.top())
        : stop_level;

      stop_level = stop_level == NO_LABEL || level_cmp_lt<level_comp_t>(overflow_level, stop_level, _level_comparator)
        ? overflow_level
        : stop_level;

      const bool has_stop_level = stop_level != NO_LABEL;

      adiar_debug(has_next_level(),
                  "There should be a next level to go to");

      adiar_debug(!has_stop_level || !has_front_bucket()
                  || level_cmp_lt<level_comp_t>(front_bucket_level(), stop_level, _level_comparator),
                  "'stop_level' should be past the current front bucket (if it exists)");

      adiar_debug(!has_front_bucket() ||
                  level_cmp_lt<level_comp_t>(front_bucket_level(), back_bucket_level(), _level_comparator),
                  "Back bucket should be (strictly) ahead of the back bucket");

      // TODO: Add statistics on what case is hit.

      // Edge Case: ---------------------------------------------------------- :
      //   The given stop_level is prior to the next bucket
      if (has_stop_level && level_cmp_lt<level_comp_t>(stop_level, next_bucket_level(), _level_comparator)) {
        return;
      }

      // Edge Case: ---------------------------------------------------------- :
      //   All buckets are empty, so we can merely relabel them and not pay for
      //   any (re)initialisation. Furthermore, we can have one more bucket
      //   ready to 'catch' the next elements.
      if (size() == _overflow_queue.size()) {
        adiar_debug(has_stop_level, "Must have a 'stop_level' to go to");

        relabel_buckets(stop_level);
        return;
      }

      // Primary Case: ------------------------------------------------------- :
      //   At least one bucket contains an element. Let us go through each
      //   bucket one-by-one until we find the one or hit the stop_level.
      forward_to_nonempty_bucket(stop_level, has_stop_level);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the current level is empty. Is false, if there is no
    ///       'current' level.
    ////////////////////////////////////////////////////////////////////////////
    bool empty_level()
    {
      // TODO: change semantics to require 'has_current_level'
      return !has_current_level() ||
        (// Do we not have any cached element from top()?
         !_has_top_elem
         // and the current bucket is empty?
         && !_has_next_from_bucket
         // and the priority queue has nothing for this level?
         && (_overflow_queue.empty()
             || current_level() != elem_level_t::label_of(_overflow_queue.top()) ) );
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether any elements can be pulled (from the current level).
    ///
    /// \sa    levelized_priority_queue::empty_level
    ////////////////////////////////////////////////////////////////////////////
    bool can_pull()
    {
      return !empty_level();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the top element of the current level.
    ////////////////////////////////////////////////////////////////////////////
    elem_t top()
    {
      adiar_debug (can_pull(), "Can only obtain top element on non-empty level");

      if (!_has_top_elem) {
        _top_elem = pull();
        _has_top_elem = true;
      }

      return _top_elem;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the top element of the current level.
    ///
    /// \sa    levelized_priority_queue::top
    ////////////////////////////////////////////////////////////////////////////
    elem_t peek()
    {
      return top();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the top element on the current level and remove it.
    ////////////////////////////////////////////////////////////////////////////
    elem_t pull()
    {
      adiar_debug (!empty_level(), "Can only pull on non-empty level");

      if (_has_top_elem) {
        _has_top_elem = false;
        return _top_elem;
      }

      adiar_debug (_size > 0, "pull on non-top element requires content");
      _size--;

      // Merge bucket with overflow queue
      if (_overflow_queue.empty() || (_has_next_from_bucket
                                      && _e_comparator(_next_from_bucket, _overflow_queue.top()))) {
        const elem_t ret = _next_from_bucket;
        if (_buckets_sorter[_front_bucket_idx] -> can_pull()) {
          _next_from_bucket = _buckets_sorter[_front_bucket_idx] -> pull();
        } else {
          _has_next_from_bucket = false;
        }
        return ret;
      } else {
        const elem_t ret = _overflow_queue.top();
        _overflow_queue.pop();
        return ret;
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Remove the top element of the current level.
    ///
    /// \sa    levelized_priority_queue::pull
    ////////////////////////////////////////////////////////////////////////////
    void pop()
    {
      pull();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The total number of elements (across all levels).
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      // TODO: also separate '_size' away from the overflow queue?
      return _size + _has_top_elem;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Whether the entire priority queue (across all levels) is empty.
    ///
    /// \remark If you only want to know if it is empty for the current level,
    ///         then use <tt>empty_level</tt> (or <tt>can_pull</tt>) instead.
    ///
    /// \sa     levelized_priority_queue::empty_level
    //          levelized_priority_queue::can_pull
    ////////////////////////////////////////////////////////////////////////////
    bool empty() const
    {
      return size() == 0;
    }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief  The number of active buckets, incl. the (possibly existing)
    ///         read-only bucket.
    ///
    /// \remark To compute the number of pushable buckets, please write
    ///         <tt>active_buckets() - has_front_bucket()</tt>.
    ////////////////////////////////////////////////////////////////////////////
    size_t active_buckets() const
    {
      if (_front_bucket_idx == OUT_OF_BUCKETS_IDX) {
        return _back_bucket_idx + 1;
      }

      return _front_bucket_idx <= _back_bucket_idx
        ? (_back_bucket_idx - _front_bucket_idx) + 1
        : (BUCKETS - _front_bucket_idx) + _back_bucket_idx + 1;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether more buckets are available.
    ////////////////////////////////////////////////////////////////////////////
    bool has_next_bucket() const
    {
      return _front_bucket_idx != _back_bucket_idx;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Label of the next bucket.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_t next_bucket_level() const
    {
      adiar_debug(has_next_bucket(),
                  "Cannot obtain level of non-existing next bucket");

      const ptr_uint64::label_t next_idx   = (_front_bucket_idx + 1) % BUCKETS;
      const ptr_uint64::label_t next_level = _buckets_level[next_idx];
      return next_level;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there is an 'active' bucket from which ca be pulled
    ///        (though it may need to be sorted).
    ////////////////////////////////////////////////////////////////////////////
    bool has_front_bucket() const
    {
      return _front_bucket_idx != OUT_OF_BUCKETS_IDX;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Level of the front bucket.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_t front_bucket_level() const
    {
      return _buckets_level[_front_bucket_idx];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Level of the back bucket.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_t back_bucket_level() const
    {
      return _buckets_level[_back_bucket_idx];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Forwards to the first non-empty bucket or at the given
    ///        'stop_level'.
    ///
    /// \param stop_level     The forwarding will stop early if reaching this
    ///                       level.
    ///
    /// \param has_stop_level Whether to take the stop_level parameter into
    ///                       account.
    ///
    /// \sa    levelized_priority_queue::setup_next_level
    ////////////////////////////////////////////////////////////////////////////
    inline void forward_to_nonempty_bucket(const ptr_uint64::label_t stop_level, const bool has_stop_level)
    {
      do {
        adiar_invariant(has_next_bucket(),
                        "At least one more bucket can be forwarded to");

        // Is the next bucket past the 'stop_level'?
        if (has_stop_level && level_cmp_lt<level_comp_t>(stop_level, next_bucket_level(), _level_comparator)) {
          break;
        }

        adiar_invariant(!has_front_bucket()
                        || level_cmp_lt<level_comp_t>(front_bucket_level(), back_bucket_level(), _level_comparator),
                        "Inconsistency in has_next_bucket predicate");

        // Replace the current read-only bucket, if there is one
        if (_level_merger.can_pull() && has_front_bucket()) {
          const ptr_uint64::label_t next_level = _level_merger.pull();

          _buckets_level[_front_bucket_idx] = next_level;
          sorter_t::reset_unique(_buckets_sorter[_front_bucket_idx],
                                 _memory_for_buckets, _max_size, BUCKETS);

          _back_bucket_idx = _front_bucket_idx;
        }
        _front_bucket_idx = (_front_bucket_idx + 1) % BUCKETS;

        adiar_debug(!has_next_bucket() || !has_front_bucket()
                    || level_cmp_lt<level_comp_t>(front_bucket_level(), back_bucket_level(), _level_comparator),
                    "Inconsistency in has_next_bucket predicate");

        adiar_debug(has_next_bucket() || !has_front_bucket()
                    || front_bucket_level() == back_bucket_level(),
                    "Inconsistency in has_next_bucket predicate");

        // Sort front bucket
        _buckets_sorter[_front_bucket_idx] -> sort();

        _has_next_from_bucket = _buckets_sorter[_front_bucket_idx] -> can_pull();
        if (_has_next_from_bucket) {
          _next_from_bucket = _buckets_sorter[_front_bucket_idx] -> pull();
        }
      } while (!_has_next_from_bucket && has_next_bucket());

      _current_level = front_bucket_level();

      adiar_debug(has_front_bucket(), "Ends with a front bucket");

      adiar_debug((has_stop_level
                   && (level_cmp_le<level_comp_t>(stop_level, front_bucket_level(), _level_comparator)
                       || (!has_next_bucket() || level_cmp_lt<level_comp_t>(stop_level, next_bucket_level(), _level_comparator))) )
                  || _has_next_from_bucket,
                  "Either we stopped early or we found a non-bucket");

      adiar_debug(level_cmp_le<level_comp_t>(front_bucket_level(), back_bucket_level(), _level_comparator),
                  "Consistent bucket levels");
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Relabel all buckets to be a level at or past the stop_level. If
    ///        the current read-only bucket is relabelled, then it is also reset
    ///        to be pushable.
    ///
    /// \sa    levelized_priority_queue::setup_next_level
    ////////////////////////////////////////////////////////////////////////////
    inline void relabel_buckets(const ptr_uint64::label_t stop_level)
    {
      adiar_debug(stop_level != NO_LABEL,
                  "Relabelling of buckets require a valid 'stop_level'");

      // Backup of start and end of circular array
      const size_t old_front_bucket_idx = _front_bucket_idx;
      const size_t old_back_bucket_idx = _back_bucket_idx;

      // Create a list of the new levels
      ptr_uint64::label_t new_levels[BUCKETS];
      _back_bucket_idx = OUT_OF_BUCKETS_IDX;

      // Copy over still relevant levels from current buckets
      do {
        _front_bucket_idx = (_front_bucket_idx + 1) % BUCKETS;

        adiar_debug(has_front_bucket(), "After increment the front bucket will 'exist'");

        if (level_cmp_le<level_comp_t>(front_bucket_level(), stop_level, _level_comparator)) {
          _current_level = front_bucket_level();
        } else { // level_cmp_lt<level_comp_t>(stop_level, front_bucket_level(), _level_comparator)
          new_levels[++_back_bucket_idx] = front_bucket_level();
        }
      } while (_front_bucket_idx != old_back_bucket_idx);

      _front_bucket_idx = OUT_OF_BUCKETS_IDX;

      // Add as many levels from the level_merger as we can fit in
      while (_level_merger.can_pull() && level_cmp_le<level_comp_t>(_level_merger.peek(), stop_level, _level_comparator)) {
        _current_level = _level_merger.pull();
      }

      while (_back_bucket_idx + 1 < BUCKETS && _level_merger.can_pull()) {
        new_levels[++_back_bucket_idx] = _level_merger.pull();
      }

      adiar_debug(_back_bucket_idx == OUT_OF_BUCKETS_IDX || _back_bucket_idx < BUCKETS,
                  "_back_bucket_idx is a valid index");

      // Relabel all buckets
      if (_back_bucket_idx != OUT_OF_BUCKETS_IDX) {
        for (size_t idx = 0; idx <= _back_bucket_idx; idx++) {
          _buckets_level[idx] = new_levels[idx];
        }

        // Reset the prior read-only bucket, if relevant
        if (old_front_bucket_idx <= _back_bucket_idx) {
          sorter_t::reset_unique(_buckets_sorter[old_front_bucket_idx],
                                 _memory_for_buckets, _max_size, BUCKETS);
        }

        // We can clean up all the dead buckets with a '.reset()' on the
        // unique_ptr, but we will leave that for the destructor to do.
      }
    }
  };

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Specialization of the levelized priority queue with a look_ahead
  ///        of '0', i.e. **without** any buckets.
  ////////////////////////////////////////////////////////////////////////////
  template <typename elem_t,
            typename elem_level_t,
            typename elem_comp_t,
            memory_mode_t mem_mode,
            typename file_t,
            size_t   FILES,
            typename level_comp_t,
            ptr_uint64::label_t  INIT_LEVEL
            >
  class levelized_priority_queue<elem_t, elem_level_t, elem_comp_t, 0u, // <--
                                 mem_mode, file_t, FILES, level_comp_t, INIT_LEVEL>
  {
  private:
    static constexpr ptr_uint64::label_t NO_LABEL = ptr_uint64::MAX_LABEL+1;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the overflow priority queue.
    ////////////////////////////////////////////////////////////////////////////
    typedef priority_queue<mem_mode, elem_t, elem_comp_t> priority_queue_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Total number of data structures in Levelized Priority Queue.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr size_t DATA_STRUCTURES = priority_queue_t::DATA_STRUCTURES;

  public:
    static tpie::memory_size_type memory_usage(tpie::memory_size_type no_elements)
    {
      return priority_queue<memory_mode_t::INTERNAL, elem_t, elem_comp_t>::memory_usage(no_elements);
    }

    static tpie::memory_size_type memory_fits(tpie::memory_size_type memory_bytes)
    {
      return priority_queue<memory_mode_t::INTERNAL, elem_t, elem_comp_t>::memory_fits(memory_bytes);
    }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Instantiation of the comparator between levels.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_t _current_level = NO_LABEL;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Instantiation of the comparator between levels.
    ////////////////////////////////////////////////////////////////////////////
    level_comp_t _level_comparator = level_comp_t();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Instantiation of the comparator between elements.
    ////////////////////////////////////////////////////////////////////////////
    elem_comp_t _e_comparator = elem_comp_t();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Maximum size of levelized priority queue.
    ////////////////////////////////////////////////////////////////////////////
    const size_t _max_size;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Total memory given in constructor.
    ////////////////////////////////////////////////////////////////////////////
    const tpie::memory_size_type _memory_given;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Overflow priority queue used for elements pushed to a level where
    ///        a bucket is (yet) not created.
    ////////////////////////////////////////////////////////////////////////////
    priority_queue_t _priority_queue;

#ifdef ADIAR_STATS
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The actual maximum size of the levelized priority queue.
    ////////////////////////////////////////////////////////////////////////////
    size_t _actual_max_size = 0u;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Reference to struct to store non-global stats into.
    ////////////////////////////////////////////////////////////////////////////
    stats_t::levelized_priority_queue_t &_stats;
#endif

  private:
    levelized_priority_queue(tpie::memory_size_type memory_given, size_t max_size,
                             [[maybe_unused]] stats_t::levelized_priority_queue_t &stats)
      : _max_size(max_size),
        _memory_given(memory_given),
        _priority_queue(memory_given, max_size)
#ifdef ADIAR_STATS
      , _stats(stats)
#endif
    { }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief              Instantiate with the given amount of memory.
    ///
    /// \param files        Files to follow the levels of
    ///
    /// \param memory_given Total amount of memory to use
    ////////////////////////////////////////////////////////////////////////////
    levelized_priority_queue(const file_t (& /*files*/) [FILES],
                             tpie::memory_size_type memory_given,
                             size_t max_size,
                             stats_t::levelized_priority_queue_t &stats)
      : levelized_priority_queue(memory_given, max_size, stats)
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief              Instantiate with the given amount of memory.
    ///
    /// \param dds          Decision Diagrams to follow the levels of
    ///
    /// \param memory_given Total amount of memory to use
    ////////////////////////////////////////////////////////////////////////////
    levelized_priority_queue(const dd (& /*dds*/) [FILES],
                             tpie::memory_size_type memory_given,
                             size_t max_size,
                             stats_t::levelized_priority_queue_t &stats)
      : levelized_priority_queue(memory_given, max_size, stats)
    { }

  public:
    ~levelized_priority_queue()
    {
#ifdef ADIAR_STATS
      stats_levelized_priority_queue.sum_predicted_max_size += _max_size;
      _stats.sum_predicted_max_size += _max_size;

      stats_levelized_priority_queue.sum_actual_max_size += _actual_max_size;
      _stats.sum_actual_max_size += _actual_max_size;

      stats_levelized_priority_queue.sum_max_size_ratio += frac(_actual_max_size, _max_size);
      _stats.sum_max_size_ratio += frac(_actual_max_size, _max_size);

      stats_levelized_priority_queue.sum_destructors++;
      _stats.sum_destructors++;
#endif
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there is any current level to pull elements from.
    ////////////////////////////////////////////////////////////////////////////
    bool has_current_level() const
    {
      return _current_level != NO_LABEL;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The label of the current level.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_t current_level()
    {
      adiar_debug(has_current_level(),
                  "Needs to have a 'current' level to read the level from");

      return _current_level;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there are any more (possibly all empty) levels.
    ////////////////////////////////////////////////////////////////////////////
    bool has_next_level() /*const*/
    {
      if(_priority_queue.empty()) {
        return false;
      }
      ptr_uint64::label_t next_label_from_queue = elem_level_t::label_of(_priority_queue.top());
      return (has_current_level() && level_cmp_lt<level_comp_t>(_current_level, next_label_from_queue, _level_comparator))
              || (!has_current_level() && !_priority_queue.empty());
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The label of the next (possibly empty) level.
    ////////////////////////////////////////////////////////////////////////////
    ptr_uint64::label_t next_level() /*const*/
    {
      return elem_level_t::label_of(_priority_queue.top());
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether one can push elements.
    ////////////////////////////////////////////////////////////////////////////
    bool can_push() /*const*/
    {
      return true;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Push an element into the priority queue.
    ////////////////////////////////////////////////////////////////////////////
    void push(const elem_t &e)
    {
#ifdef ADIAR_STATS
      _actual_max_size = std::max(_actual_max_size, _priority_queue.size());
#endif
      _priority_queue.push(e);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief            Set up the next nonempty level to which some requests
    ///                   have been pushed before the given stop_level.
    ///
    /// \param stop_level The level that should be furthest forwarded to. If no
    ///                   stop level is given then the next level is based on
    ///                   the next existing element in the priority queue.
    ////////////////////////////////////////////////////////////////////////////
    void setup_next_level(ptr_uint64::label_t stop_level = NO_LABEL)
    {
      adiar_debug(stop_level <= ptr_uint64::MAX_LABEL || stop_level == NO_LABEL,
                  "The stop level should be a legal value (or not given)");

      adiar_debug(!has_current_level() || empty_level(),
                  "Level is empty before moving on to the next");

      const bool has_stop_level = stop_level != NO_LABEL;

      adiar_debug(has_stop_level || !empty(),
                  "Either a stop level is given or we have some non-empty level to forward to");

      // Edge Case: ---------------------------------------------------------- :
      //   The given stop_level is prior to the next level or there is nothing in the queue
      if ((has_stop_level && level_cmp_lt<level_comp_t>(stop_level, next_level(), _level_comparator)) || _priority_queue.empty()) {
        _current_level = stop_level;
        return;
      }

      // Edge Case: ---------------------------------------------------------- :
      //   The stop level is before the next level of the queue
      adiar_debug(has_next_level(),
                  "There should be a next level to go to");
      ptr_uint64::label_t next_level_from_queue = next_level();
      if(has_stop_level && level_cmp_le<level_comp_t>(stop_level, next_level_from_queue, _level_comparator)) {
        _current_level = stop_level;
        return;
      }

      // Primary Case: ------------------------------------------------------- :
      //   Set the level to be the next from the queue
      _current_level = next_level_from_queue;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the current level is empty. Is false, if there is no
    ///       'current' level.
    ////////////////////////////////////////////////////////////////////////////
    bool empty_level()
    {
      // TODO: change semantics to require 'has_current_level'
      return !has_current_level() ||
             _priority_queue.empty() ||
             current_level() != elem_level_t::label_of(_priority_queue.top());
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether any elements can be pulled (from the current level).
    ///
    /// \sa    levelized_priority_queue::empty_level
    ////////////////////////////////////////////////////////////////////////////
    bool can_pull()
    {
      return !empty_level();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the top element of the current level.
    ////////////////////////////////////////////////////////////////////////////
    elem_t top()
    {
      adiar_debug (can_pull(), "Can only obtain top element on non-empty level");
      return _priority_queue.top();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the top element of the current level.
    ///
    /// \sa    levelized_priority_queue::top
    ////////////////////////////////////////////////////////////////////////////
    elem_t peek()
    {
      return top();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the top element on the current level and remove it.
    ////////////////////////////////////////////////////////////////////////////
    elem_t pull()
    {
      adiar_debug (!empty_level(), "Can only pull on non-empty level");

      const elem_t ret = _priority_queue.top();
      _priority_queue.pop();
      return ret;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Remove the top element of the current level.
    ///
    /// \sa    levelized_priority_queue::pull
    ////////////////////////////////////////////////////////////////////////////
    void pop()
    {
      pull();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The total number of elements (across all levels).
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      return _priority_queue.size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Whether the entire priority queue (across all levels) is empty.
    ///
    /// \remark If you only want to know if it is empty for the current level,
    ///         then use <tt>empty_level</tt> (or <tt>can_pull</tt>) instead.
    ///
    /// \sa     levelized_priority_queue::empty_level
    //          levelized_priority_queue::can_pull
    ////////////////////////////////////////////////////////////////////////////
    bool empty() const
    {
      return _priority_queue.empty();
    }
  };

  template <typename elem_t,
            typename elem_level_t,
            typename elem_comp_t         = std::less<elem_t>,
            ptr_uint64::label_t  LOOK_AHEAD          = ADIAR_LPQ_LOOKAHEAD,
            memory_mode_t mem_mode = memory_mode_t::EXTERNAL,
            size_t   FILES       = 1u,
            ptr_uint64::label_t  INIT_LEVEL  = 1u>
  using levelized_node_priority_queue = levelized_priority_queue<elem_t, elem_level_t,
                                                                 elem_comp_t, LOOK_AHEAD,
                                                                 mem_mode,
                                                                 node_file, FILES, std::less<ptr_uint64::label_t>,
                                                                 INIT_LEVEL>;

  template <typename elem_t,
            typename elem_level_t,
            typename elem_comp_t         = std::less<elem_t>,
            ptr_uint64::label_t  LOOK_AHEAD          = ADIAR_LPQ_LOOKAHEAD,
            memory_mode_t mem_mode = memory_mode_t::EXTERNAL,
            size_t   FILES               = 1u,
            ptr_uint64::label_t  INIT_LEVEL          = 1u>
  using levelized_arc_priority_queue = levelized_priority_queue<elem_t, elem_level_t,
                                                                elem_comp_t, LOOK_AHEAD,
                                                                mem_mode,
                                                                arc_file, FILES, std::greater<ptr_uint64::label_t>,
                                                                INIT_LEVEL>;

  template <typename elem_t,
            typename elem_level_t,
            typename elem_comp_t         = std::less<elem_t>,
            ptr_uint64::label_t  LOOK_AHEAD          = ADIAR_LPQ_LOOKAHEAD,
            memory_mode_t mem_mode = memory_mode_t::EXTERNAL,
            size_t   FILES               = 1u,
            ptr_uint64::label_t  INIT_LEVEL          = 1u>
  using levelized_label_priority_queue = levelized_priority_queue<elem_t, elem_level_t,
                                                                  elem_comp_t, LOOK_AHEAD,
                                                                  mem_mode,
                                                                  label_file, FILES, std::less<ptr_uint64::label_t>,
                                                                  INIT_LEVEL>;
}

#endif // ADIAR_INTERNAL_DATA_STRUCTURES_LEVELIZED_PRIORITY_QUEUE_H

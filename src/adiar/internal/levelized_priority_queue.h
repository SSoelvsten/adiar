#ifndef ADIAR_INTERNAL_LEVELIZED_PRIORITY_QUEUE_H
#define ADIAR_INTERNAL_LEVELIZED_PRIORITY_QUEUE_H

#include <limits>

#include <tpie/tpie.h>

#include <tpie/file.h>
#include <tpie/file_stream.h>
#include <tpie/sort.h>

#include <adiar/data.h>
#include <adiar/file.h>
#include <adiar/file_stream.h>

#include <adiar/internal/decision_diagram.h>
#include <adiar/internal/priority_queue.h>
#include <adiar/internal/sorter.h>
#include <adiar/internal/util.h>

#include <adiar/statistics.h>

namespace adiar {
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
    static constexpr size_t memory_usage()
    {
      return FILES * stream_t::memory_usage();
    }

  private:
    comp_t _comparator = comp_t();

    std::unique_ptr<stream_t> _label_streams [FILES];

  public:
    void hook(const file_t (&fs) [FILES])
    {
      for (size_t idx = 0u; idx < FILES; idx++) {
        _label_streams[idx] = std::make_unique<stream_t>(fs[idx]);
      }
    }

    void hook(const decision_diagram (&dds) [FILES])
    {
      for (size_t idx = 0u; idx < FILES; idx++) {
        _label_streams[idx] = std::make_unique<stream_t>(dds[idx].file);
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

    label_t peek()
    {
      adiar_debug(can_pull(),
                  "Cannot peek past end of all streams");

      bool has_min_label = false;
      label_t min_label = 0u;
      for (size_t idx = 0u; idx < FILES; idx++) {
        if (_label_streams[idx] -> can_pull()
            && (!has_min_label || _comparator(__label_of<>(_label_streams[idx] -> peek()), min_label))) {
          has_min_label = true;
          min_label = __label_of<>(_label_streams[idx] -> peek());
        }
      }

      return min_label;
    }

    label_t pull()
    {
      adiar_debug(can_pull(),
                  "Cannot pull past end of all streams");

      label_t min_label = peek();

      // pull from all with min_label
      for (const std::unique_ptr<stream_t> &level_info_stream : _label_streams) {
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
  extern stats_t::priority_queue_t stats_priority_queue;

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
            typename elem_comp_t  = std::less<elem_t>,
            template<typename, typename> typename sorter_template = external_sorter,
            template<typename, typename> typename priority_queue_template = external_priority_queue,
            typename file_t       = meta_file<elem_t>,
            size_t   FILES        = 1u,
            typename level_comp_t = std::less<label_t>,
            label_t  INIT_LEVEL   = 1u,
            label_t  LOOK_AHEAD   = ADIAR_LPQ_LOOKAHEAD
            >
  class levelized_priority_queue
  {
  private:
    static_assert(0 < LOOK_AHEAD,
                  "LOOK_AHEAD must at least be of one level");

    // TODO: LOOK_AHEAD must be strictly smaller than MAX_LABEL (but we need to
    //       close #164 first to do this check at compile time)

    static constexpr label_t OUT_OF_BUCKETS_IDX = static_cast<label_t>(-1);

    static_assert(LOOK_AHEAD + 1 < OUT_OF_BUCKETS_IDX,
                  "LOOK_AHEAD must not be so large to also include '-1'");

    static_assert(OUT_OF_BUCKETS_IDX + 1 == 0,
                  "Overflow to '0' is necessary for internal logic to work");

    static constexpr label_t NO_LABEL = MAX_LABEL+1;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the sorter for each bucket.
    ////////////////////////////////////////////////////////////////////////////
    typedef sorter_template<elem_t, elem_comp_t> sorter_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Type of the overflow priority queue.
    ////////////////////////////////////////////////////////////////////////////
    typedef priority_queue_template<elem_t, elem_comp_t> priority_queue_t;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of buckets.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr size_t BUCKETS = LOOK_AHEAD + 1;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Total number of data structures in Levelized Priority Queue.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr size_t DATA_STRUCTURES =
      BUCKETS * sorter_t::DATA_STRUCTURES + priority_queue_t::DATA_STRUCTURES;

  public:
    static constexpr tpie::memory_size_type memory_usage(tpie::memory_size_type no_elements)
    {
      return internal_priority_queue<elem_t, elem_comp_t>::memory_usage(no_elements)
        + BUCKETS * internal_sorter<elem_t, elem_comp_t>::memory_usage(no_elements)
        + label_merger<file_t, level_comp_t, FILES>::memory_usage();
    }

    static constexpr tpie::memory_size_type memory_fits(tpie::memory_size_type memory_bytes)
    {
      const size_t const_memory_bytes = label_merger<file_t, level_comp_t, FILES>::memory_usage();

      if (memory_bytes < const_memory_bytes) {
        return 0u;
      }

      // HACK: the 'internal_priority_queue' can take (two) fewer elements than
      // the 'internal_sorter'. So, this is a slight under-approximation of what
      // we truly could do with this amount of memory.
      return internal_priority_queue<elem_t, elem_comp_t>
        ::memory_fits((memory_bytes - const_memory_bytes) / DATA_STRUCTURES);
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
    label_t _current_level = NO_LABEL;

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
    tpie::memory_size_type _memory_occupied_by_merger;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Memory to be used for the buckets.
    ////////////////////////////////////////////////////////////////////////////
    tpie::memory_size_type _memory_for_buckets;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Memory to be used for the overflow priority queue.
    ////////////////////////////////////////////////////////////////////////////
    tpie::memory_size_type _memory_occupied_by_overflow;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Provides the levels to distribute all elements across.
    ////////////////////////////////////////////////////////////////////////////
    label_merger<file_t, level_comp_t, FILES> _level_merger;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Level of each bucket.
    ////////////////////////////////////////////////////////////////////////////
    label_t _buckets_level [BUCKETS];

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Sorter for each bucket.
    ////////////////////////////////////////////////////////////////////////////
    std::unique_ptr<sorter_t> _buckets_sorter [BUCKETS];

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Index of the currently read-from bucket (if any).
    ////////////////////////////////////////////////////////////////////////////
    label_t _front_bucket_idx = OUT_OF_BUCKETS_IDX;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Index of the last available bucket (if any).
    ////////////////////////////////////////////////////////////////////////////
    label_t _back_bucket_idx  = OUT_OF_BUCKETS_IDX;

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

#ifdef ADIAR_STATS_EXTRA
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The actual maximum size of the levelized priority queue.
    ////////////////////////////////////////////////////////////////////////////
    size_t _actual_max_size = 0u;
#endif

  private:
    static tpie::memory_size_type m_overflow_queue(tpie::memory_size_type memory_given)
    {
      const tpie::memory_size_type eight_MiB = 8 * 1024;
      const tpie::memory_size_type weighted_share = memory_given / (4 * BUCKETS + 1);

      return std::max(eight_MiB, weighted_share);
    }

    levelized_priority_queue(tpie::memory_size_type memory_given, size_t max_size)
      : _max_size(max_size),
        _memory_given(memory_given),
        _memory_occupied_by_merger(memory::available()),
        _memory_occupied_by_overflow(m_overflow_queue(memory_given)),
        _overflow_queue(m_overflow_queue(memory_given), max_size)
    { }


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
                             size_t max_size)
      : levelized_priority_queue(memory_given, max_size)
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
    levelized_priority_queue(const decision_diagram (& dds) [FILES],
                             tpie::memory_size_type memory_given,
                             size_t max_size)
      : levelized_priority_queue(memory_given, max_size)
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
      // This was set in the private constructor above to be the total amount of
      // memory. This was done before the _level_merger had created all of its
      // level_info streams, so we can get how much space they already took of
      // what we are given now.
      _memory_occupied_by_merger -= memory::available();
      _memory_for_buckets = _memory_given - _memory_occupied_by_merger - _memory_occupied_by_overflow;

      adiar_debug(_memory_occupied_by_merger + _memory_occupied_by_overflow <= _memory_given,
                  "the amount of memory used should be within the given bounds");

      // Initially skip the number of levels
      for (label_t idx = 0; _level_merger.can_pull() && idx < INIT_LEVEL; idx++) {
        _level_merger.pull();
      }

      // Set up buckets until no levels are left or all buckets have been
      // instantiated. Notice, that _back_bucket_idx was initialised to -1.
      while (_back_bucket_idx + 1 < BUCKETS && _level_merger.can_pull()) {
        const label_t level = _level_merger.pull();

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
#ifdef ADIAR_STATS_EXTRA
      stats_priority_queue.sum_predicted_max_size += _max_size;
      stats_priority_queue.sum_actual_max_size += _actual_max_size;

      stats_priority_queue.sum_max_size_ratio += frac(_actual_max_size, _max_size);
      stats_priority_queue.sum_destructors++;
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
    label_t current_level()
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
    label_t next_level() const
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

      const label_t level = elem_level_t::label_of(e);

      adiar_debug(level_cmp_le(next_bucket_level(), level),
                  "Can only push element to next bucket or later.");

      const size_t pushable_buckets = active_buckets() - has_front_bucket();

      adiar_debug(pushable_buckets > 0,
                  "There is at least one pushable bucket (i.e. level)");

      _size++;
#ifdef ADIAR_STATS_EXTRA
      _actual_max_size = std::max(_actual_max_size, _size);
#endif

      label_t bucket_offset = 1u;
      do {
        const label_t bucket_idx = (_front_bucket_idx + bucket_offset++) % BUCKETS;

        if (_buckets_level[bucket_idx] == level) {
          _buckets_sorter[bucket_idx] -> push(e);
#ifdef ADIAR_STATS_EXTRA
          stats_priority_queue.push_bucket++;
#endif
          return;
        }
      } while (bucket_offset <= pushable_buckets);

#ifdef ADIAR_STATS_EXTRA
      stats_priority_queue.push_overflow++;
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
    void setup_next_level(label_t stop_level = NO_LABEL)
    {
      adiar_debug(stop_level <= MAX_LABEL || stop_level == NO_LABEL,
                  "The stop level should be a legal value (or not given)");

      adiar_debug(!has_current_level() || empty_level(),
                  "Level is empty before moving on to the next");

      adiar_debug(stop_level != NO_LABEL || !empty(),
                  "Either a stop level is given or we have some non-empty level to forward to");

      const label_t overflow_level = !_overflow_queue.empty()
        ? elem_level_t::label_of(_overflow_queue.top())
        : stop_level;

      stop_level = stop_level == NO_LABEL || level_cmp_lt(overflow_level, stop_level)
        ? overflow_level
        : stop_level;

      const bool has_stop_level = stop_level != NO_LABEL;

      adiar_debug(has_next_level(),
                  "There should be a next level to go to");

      adiar_debug(!has_stop_level || !has_front_bucket()
                  || level_cmp_lt(front_bucket_level(), stop_level),
                  "'stop_level' should be past the current front bucket (if it exists)");

      adiar_debug(!has_front_bucket() ||
                  level_cmp_lt(front_bucket_level(), back_bucket_level()),
                  "Back bucket should be (strictly) ahead of the back bucket");

      // TODO: Add statistics on what case is hit.

      // Edge Case: ---------------------------------------------------------- :
      //   The given stop_level is prior to the next bucket
      if (has_stop_level && level_cmp_lt(stop_level, next_bucket_level())) {
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
    /// \brief Strictly less ' < ' between two levels.
    ////////////////////////////////////////////////////////////////////////////
    bool level_cmp_lt(const label_t l1, const label_t l2)
    {
      return _level_comparator(l1, l2);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Less or equal ' <= ' between two levels.
    ////////////////////////////////////////////////////////////////////////////
    bool level_cmp_le(const label_t l1, const label_t l2)
    {
      return _level_comparator(l1, l2) || l1 == l2;
    }

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
    label_t next_bucket_level() const
    {
      adiar_debug(has_next_bucket(),
                  "Cannot obtain level of non-existing next bucket");

      const label_t next_idx   = (_front_bucket_idx + 1) % BUCKETS;
      const label_t next_level = _buckets_level[next_idx];
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
    label_t front_bucket_level() const
    {
      return _buckets_level[_front_bucket_idx];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Level of the back bucket.
    ////////////////////////////////////////////////////////////////////////////
    label_t back_bucket_level() const
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
    inline void forward_to_nonempty_bucket(const label_t stop_level, const bool has_stop_level)
    {
      do {
        adiar_invariant(has_next_bucket(),
                        "At least one more bucket can be forwarded to");

        // Is the next bucket past the 'stop_level'?
        if (has_stop_level && level_cmp_lt(stop_level, next_bucket_level())) {
          break;
        }

        adiar_invariant(!has_front_bucket()
                        || level_cmp_lt(front_bucket_level(), back_bucket_level()),
                        "Inconsistency in has_next_bucket predicate");

        // Replace the current read-only bucket, if there is one
        if (_level_merger.can_pull() && has_front_bucket()) {
          const label_t next_level = _level_merger.pull();

          _buckets_level[_front_bucket_idx] = next_level;
          sorter_t::reset_unique(_buckets_sorter[_front_bucket_idx],
                                 _memory_for_buckets, _max_size, BUCKETS);

          _back_bucket_idx = _front_bucket_idx;
        }
        _front_bucket_idx = (_front_bucket_idx + 1) % BUCKETS;

        adiar_debug(!has_next_bucket() || !has_front_bucket()
                    || level_cmp_lt(front_bucket_level(), back_bucket_level()),
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
                   && (level_cmp_le(stop_level, front_bucket_level())
                       || (!has_next_bucket() || level_cmp_lt(stop_level, next_bucket_level()))) )
                  || _has_next_from_bucket,
                  "Either we stopped early or we found a non-bucket");

      adiar_debug(level_cmp_le(front_bucket_level(), back_bucket_level()),
                  "Consistent bucket levels");
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Relabel all buckets to be a level at or past the stop_level. If
    ///        the current read-only bucket is relabelled, then it is also reset
    ///        to be pushable.
    ///
    /// \sa    levelized_priority_queue::setup_next_level
    ////////////////////////////////////////////////////////////////////////////
    inline void relabel_buckets(const label_t stop_level)
    {
      adiar_debug(stop_level != NO_LABEL,
                  "Relabelling of buckets require a valid 'stop_level'");

      // Backup of start and end of circular array
      const size_t old_front_bucket_idx = _front_bucket_idx;
      const size_t old_back_bucket_idx = _back_bucket_idx;

      // Create a list of the new levels
      label_t new_levels[BUCKETS];
      _back_bucket_idx = OUT_OF_BUCKETS_IDX;

      // Copy over still relevant levels from current buckets
      do {
        _front_bucket_idx = (_front_bucket_idx + 1) % BUCKETS;

        adiar_debug(has_front_bucket(), "After increment the front bucket will 'exist'");

        if (level_cmp_le(front_bucket_level(), stop_level)) {
          _current_level = front_bucket_level();
        } else { // level_cmp_lt(stop_level, front_bucket_level())
          new_levels[++_back_bucket_idx] = front_bucket_level();
        }
      } while (_front_bucket_idx != old_back_bucket_idx);

      _front_bucket_idx = OUT_OF_BUCKETS_IDX;

      // Add as many levels from the level_merger as we can fit in
      while (_level_merger.can_pull() && level_cmp_le(_level_merger.peek(), stop_level)) {
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

  template <typename elem_t,
            typename elem_level_t,
            typename elem_comp_t = std::less<elem_t>,
            template<typename, typename> typename sorter_template = external_sorter,
            template<typename, typename> typename priority_queue_template = external_priority_queue,
            size_t   FILES       = 1u,
            label_t  INIT_LEVEL  = 1u,
            label_t  LOOK_AHEAD  = ADIAR_LPQ_LOOKAHEAD>
  using levelized_node_priority_queue = levelized_priority_queue<elem_t, elem_level_t, elem_comp_t,
                                                                 sorter_template, priority_queue_template,
                                                                 node_file, FILES, std::less<label_t>,
                                                                 INIT_LEVEL,
                                                                 LOOK_AHEAD>;

  template <typename elem_t,
            typename elem_level_t,
            typename elem_comp_t  = std::less<elem_t>,
            template<typename, typename> typename sorter_template = external_sorter,
            template<typename, typename> typename priority_queue_template = external_priority_queue,
            size_t   FILES        = 1u,
            label_t  INIT_LEVEL   = 1u,
            label_t  LOOK_AHEAD   = ADIAR_LPQ_LOOKAHEAD>
  using levelized_arc_priority_queue = levelized_priority_queue<elem_t, elem_level_t, elem_comp_t,
                                                                sorter_template, priority_queue_template,
                                                                arc_file, FILES, std::greater<label_t>,
                                                                INIT_LEVEL,
                                                                LOOK_AHEAD>;

  template <typename elem_t,
            typename elem_level_t,
            typename elem_comp_t  = std::less<elem_t>,
            template<typename, typename> typename sorter_template = external_sorter,
            template<typename, typename> typename priority_queue_template = external_priority_queue,
            size_t   FILES        = 1u,
            label_t  INIT_LEVEL   = 1u,
            label_t  LOOK_AHEAD   = ADIAR_LPQ_LOOKAHEAD>
  using levelized_label_priority_queue = levelized_priority_queue<elem_t, elem_level_t, elem_comp_t,
                                                                  sorter_template, priority_queue_template,
                                                                  label_file, FILES, std::less<label_t>,
                                                                  INIT_LEVEL,
                                                                  LOOK_AHEAD>;

  //////////////////////////////////////////////////////////////////////////////
  /// TODO: Make a levelized_priority_queue that does not have any buckets
}

#endif // ADIAR_INTERNAL_LEVELIZED_PRIORITY_QUEUE_H

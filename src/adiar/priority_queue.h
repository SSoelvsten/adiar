#ifndef ADIAR_PRIORITY_QUEUE_H
#define ADIAR_PRIORITY_QUEUE_H

#include <tpie/tpie.h>

#include <tpie/file.h>
#include <tpie/file_stream.h>
#include <tpie/priority_queue.h>
#include <tpie/sort.h>

#include <adiar/data.h>
#include <adiar/file.h>
#include <adiar/file_stream.h>

#include <adiar/bdd/bdd.h>

namespace adiar {
  extern tpie::dummy_progress_indicator pq_tpie_progress_indicator;

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Conversion from intended memory to give a `tpie::priority_queue<>`
  /// to the float `factor` it takes as an argument.
  ///
  /// Example of usage:
  ///
  /// `tpie::priority_queue<...> pq(calc_tpie_pq_factor(intended_bytes)`
  float calc_tpie_pq_factor(tpie::memory_size_type memory_given);

  ////////////////////////////////////////////////////////////////////////////
  // TODO: TPIE priority queue wrapper? It can deal with the calc_tpie_pq_factor
  // function above, and given an estimate of elements within it can choose to
  // use the tpie::internal_priority_queue.

  ////////////////////////////////////////////////////////////////////////////
  // Levelized Priority Queue memory size computations
  //
  // We set up the merge_sorters such that they have a very low memory footprint
  // on Phase 1 (sending sorted blocks to disk) and Phase 3 (merging sorted
  // partial results). This allows us to place a lot of memory on Phase 2 (the
  // primary sorting step), which will allow us to have multiple concurrent
  // merger_sorters, since only one of them will every be running Phase 2, while
  // all the others are active at Phase 1 or Phase 3 at the same time.

  // TODO: Move into an adiar/memory.h file? This is similar computations as in
  //       reduce.cpp
  template<typename T>
  tpie::memory_size_type m_single_block()
  {
    // 128 Kb * sizeof(T) + 5 MB. This is the minimum to make TPIE not cry.
    return sizeof(T) * 128 * 1024 + 5 * 1024 * 1024;
  }

  template<typename T, size_t Sorters>
  tpie::memory_size_type m_overflow_queue(tpie::memory_size_type memory_given)
  {
    // At least as much as a single bucket, but otherwise only a fraction of the
    // memory. The more sorters there are the less the less it will be needed.
    return std::max(m_single_block<T>(),
                    memory_given / (/* buckets */ (Sorters+1) +
                                    /* queue itself */ 1));
  }

  template<typename T, size_t Sorters>
  tpie::memory_size_type m_sort(tpie::memory_size_type memory_given,
                                tpie::memory_size_type memory_occupied_by_meta)
  {
    // Total amount of memory minus the blocks set aside for all buckets
    return memory_given
      - (m_single_block<T>() * Sorters)
      - memory_occupied_by_meta
      - m_overflow_queue<T,Sorters>(memory_given);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// We will use a stream of labels to retrieve the values from. Since this may
  /// be one or more streams (Apply, Equality, and Relational Product, and
  /// Reduce all need to consider two input streams). Luckily, the priority
  /// queue will never have to outlive its meta stream, so we may merely just
  /// have a reference to it.
  ///
  /// What we need is a policy, that is able to merge one or more meta streams.
  /// Currently these always are to be read in reverse, so let us merely
  /// implement a manager for just that.
  //////////////////////////////////////////////////////////////////////////////
  template <typename File_T, size_t Files,
            typename Comparator = std::less<label_t>, size_t MetaStreams = 1u>
  class pq_label_mgr
  {
  private:
    Comparator _comparator = Comparator();

    size_t _files_given = 0;

    // Notice, that this will break, if the original file is garbage collected
    // before the priority queue. But, currently the original file always is an
    // argument to the function, in which this pq_label_mgr lives within.
    std::unique_ptr<meta_stream<File_T, Files>> _meta_streams [MetaStreams];

  public:
    bool hook_meta_stream(const meta_file<File_T, Files> &f)
    {
      adiar_debug(_files_given < MetaStreams, "Given more files than was expected");

      _meta_streams[_files_given] = std::make_unique<meta_stream<File_T, Files>>(f);

      return ++_files_given == MetaStreams;
    }

    bool can_pull()
    {
      adiar_debug(_files_given == MetaStreams,
                 "Cannot check existence of next element before being attached to all meta streams");

      for (size_t idx = 0u; idx < MetaStreams; idx++) {
        if (_meta_streams[idx] -> can_pull()) {
          return true;
        }
      }
      return false;
    }

    label_t peek()
    {
      adiar_debug(_files_given == MetaStreams,
                 "Peeking element before being attached to all expected meta streams");
      adiar_debug(can_pull(),
                 "Cannot peek past end of all streams");

      bool has_min_label = false;
      label_t min_label = 0u;
      for (size_t idx = 0u; idx < MetaStreams; idx++) {
        if (_meta_streams[idx] -> can_pull()
            && (!has_min_label || _comparator(_meta_streams[idx] -> peek().label, min_label))) {
          has_min_label = true;
          min_label = _meta_streams[idx] -> peek().label;
        }
      }

      return min_label;
    }

    label_t pull()
    {
      adiar_debug(_files_given == MetaStreams,
                 "Pulling element before being attached to all expected meta streams");
      adiar_debug(can_pull(),
                 "Cannot pull past end of all streams");

      label_t min_label = peek();

      // pull from all with min_label
      for (const std::unique_ptr<meta_stream<File_T, Files>> &meta_stream : _meta_streams) {
        if (meta_stream -> can_pull() && meta_stream -> peek().label == min_label) {
          meta_stream -> pull();
        }
      }

      return min_label;
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// The preprocessor variable ADIAR_PQ_BUCKETS can be used to change the
  /// number of buckets used by the levelized priority queue.

#ifndef ADIAR_PQ_BUCKETS
#define ADIAR_PQ_BUCKETS 1u
#endif

  //////////////////////////////////////////////////////////////////////////////
  /// A levelized priority queue for BDDs capable of improving performance by
  /// placing all pushed queue elements in buckets for the specific layer and
  /// then sorting it when one finally arrives at said layer. If no bucket
  /// exists for said request, then they will be placed in a priority queue to
  /// then be merged with the current bucket.
  ///
  /// \param T          The type of items to store.
  /// \param Files      The number of files used in the adiar::file for T
  ///
  /// \param LabelExt   Struct, that provides a .label_of(T& t) function to
  ///                   determine in which bucket to place it.
  ///
  /// \param TComparator Sorting comparator to use in the inner queue, buckets
  ///                    and for merging.
  ///
  /// \param MetaStreams Number of meta information streams about the input
  ///
  /// \param LabelComparator Comparator to be used for merging multiple meta
  ///                        information streams together.
  ///                         - std::less    : top-down labelling
  ///                         - std::greater : bottom-up labelling
  ///
  /// \Buckets          The number of buckets. If 0, then this is merely a
  ///                   wrapper for the tpie::priority_queue.
  ///
  /// This makes use of two policies to manage and place elements in the
  /// buckets.
  //////////////////////////////////////////////////////////////////////////////
  template <typename File_T, size_t Files,
            typename T,
            typename LabelExt,
            typename TComparator = std::less<T>, typename LabelComparator = std::less<label_t>,
            size_t MetaStreams = 1u, size_t Buckets = ADIAR_PQ_BUCKETS>
  class priority_queue : private LabelExt, private pq_label_mgr<File_T, Files, LabelComparator, MetaStreams>
  {
    static_assert(0 <= Buckets && Buckets <= 4, "The number of buckets may only be in [0;4]");
    static_assert(0 < MetaStreams, "At least one meta stream should be provided");

    typedef pq_label_mgr<File_T, Files, LabelComparator, MetaStreams> label_mgr;

    ////////////////////////////////////////////////////////////////////////////
    // Internal state
  private:
    size_t _size = 0;

    bool _has_peeked = false;
    T _peeked;

    LabelComparator _label_comparator = LabelComparator();
    TComparator _t_comparator = TComparator();

    size_t _front_bucket_idx = 0;
    size_t _back_bucket_idx = 0;

    label_t _buckets_label [Buckets + 1];

    tpie::memory_size_type _memory_occupied_by_meta;
    tpie::memory_size_type _buckets_memory;
    std::unique_ptr<tpie::merge_sorter<T, false, TComparator>> _buckets_sorter [Buckets + 1];

    bool _has_next_from_bucket = false;
    T _next_from_bucket;

    tpie::priority_queue<T, TComparator> _overflow_queue;

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
  private:
    priority_queue(tpie::memory_size_type memory_given)
      : _overflow_queue(calc_tpie_pq_factor(m_overflow_queue<T, Buckets>(memory_given)))
    {
      _memory_occupied_by_meta = tpie::get_memory_manager().available();
      _buckets_memory = memory_given;

      adiar_debug(m_single_block<T>() * Buckets < _buckets_memory,
                  "Not enough memory to instantiate all buckets concurrently");
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Instantiate the priority_queue with the given amount of memory.
    ///
    /// \param memory_given    Total amount of memory the priority queue should
    ///                        take.
    ////////////////////////////////////////////////////////////////////////////
    priority_queue(const meta_file<File_T, Files> (& files) [MetaStreams],
                   tpie::memory_size_type memory_given)
      : priority_queue(memory_given)
    {
      for (const meta_file<File_T, Files> &f : files) {
        label_mgr::hook_meta_stream(f);
      }
      setup_buckets();
    }

    priority_queue(const bdd (& bdds) [MetaStreams],
                   tpie::memory_size_type memory_given)
      : priority_queue(memory_given) {
      for (const bdd& b : bdds) {
        label_mgr::hook_meta_stream(b.file);
      }
      setup_buckets();
    }

    priority_queue(const meta_file<File_T, Files> (& files) [MetaStreams])
    : priority_queue(files, tpie::get_memory_manager().available()) { }

    priority_queue(const bdd (& bdds) [MetaStreams])
    : priority_queue(bdds, tpie::get_memory_manager().available()) { }

    ////////////////////////////////////////////////////////////////////////////
    // Private constructor methods
  private:
    void setup_buckets()
    {
      if (label_mgr::can_pull()) {
        _memory_occupied_by_meta -= tpie::get_memory_manager().available();

        label_t label = label_mgr::pull();
        setup_bucket(_front_bucket_idx, label);

        while(_back_bucket_idx < Buckets && label_mgr::can_pull()) {
          label_t label = label_mgr::pull();

          adiar_invariant(_label_comparator(_buckets_label[_back_bucket_idx], label), "");
          adiar_invariant(_front_bucket_idx == 0, "Front bucket not moved");
          adiar_invariant(_back_bucket_idx <= Buckets, "Buckets only created up to given limit");

          _back_bucket_idx++;
          setup_bucket(_back_bucket_idx, label);
        }

        calc_front_bucket();
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Public methods
  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Push an element into the stream
    ////////////////////////////////////////////////////////////////////////////
    void push(const T &t)
    {
      _size++;
      if constexpr (Buckets == 0) {
        return _overflow_queue.push(t);
      }

      label_t label = LabelExt::label_of(t);

      adiar_debug(_label_comparator(front_bucket_label(), label),
                 "Element pushed prior to currently active bucket");

      for (size_t bucket = 1; bucket <= Buckets && bucket <= active_buckets(); bucket++) {
        size_t bucket_idx = (_front_bucket_idx + bucket) % (Buckets + 1);
        if (_buckets_label[bucket_idx] == label) {
          _buckets_sorter[bucket_idx] -> push(t);
          return;
        }
      }
      _overflow_queue.push(t);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The label of the current layer
    ////////////////////////////////////////////////////////////////////////////
    label_t current_layer() const
    {
      if constexpr (Buckets == 0) {
        return _buckets_label[0];
      }

      return front_bucket_label();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Is there any non-empty layer?
    ////////////////////////////////////////////////////////////////////////////
    bool has_next_layer()
    {
      if constexpr (Buckets == 0) {
        adiar_debug (!can_pull(), "Cannot check on next layer on empty queue");

        return !_overflow_queue.empty();
      }

      return _size > 0 && has_next_bucket();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set up the next nonempty layer to which already some requests
    /// have been pushed.
    ////////////////////////////////////////////////////////////////////////////
    void setup_next_layer()
    {
      setup_next_layer<false>(0u);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set up the next nonempty layer before the given stop_label. If
    /// no elements are pushed to layers before stop_label, and said layer also
    /// is empty, then it will stop at said layer.
    ////////////////////////////////////////////////////////////////////////////
    void setup_next_layer(label_t stop_label)
    {
      setup_next_layer<true>(stop_label);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Are there more elements on the current layer?
    ////////////////////////////////////////////////////////////////////////////
    bool can_pull()
    {
      if constexpr (Buckets == 0) {
        return
          !_overflow_queue.empty()
          && _buckets_label[0] == LabelExt::label_of(_overflow_queue.top());
      }

      return
        _has_peeked
        // Is the current bucket non-empty?
        || _has_next_from_bucket
        // Is the priority queue non-empty and not ahead?
        || (!_overflow_queue.empty()
            && front_bucket_label() == LabelExt::label_of(_overflow_queue.top()) );
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Is the current layer of elements empty?
    ////////////////////////////////////////////////////////////////////////////
    bool empty_layer()
    {
      return !can_pull();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Peek the next element on the current layer
    ////////////////////////////////////////////////////////////////////////////
    T peek()
    {
      adiar_debug (can_pull(), "Cannot peek on empty layer/queue");

      if constexpr (Buckets == 0) {
        return _overflow_queue.top();
      }

      if (!_has_peeked) {
        _size++; // Compensate that pull() decrements the size
        _peeked = pull();
        _has_peeked = true;
      }

      return _peeked;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Alias for peek
    ////////////////////////////////////////////////////////////////////////////
    T top() {
      return peek();
    }

    void pop() {
      pull();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Pull the next element on the current layer
    ////////////////////////////////////////////////////////////////////////////
    T pull()
    {
      adiar_debug (can_pull(), "Cannot pull on empty layer/queue");

      _size--;
      if constexpr (Buckets == 0) {
        T t = _overflow_queue.top();
        _overflow_queue.pop();
        return t;
      }

      if (_has_peeked) {
        _has_peeked = false;
        return _peeked;
      }

      // Merge bucket with overflow queue
      if (_overflow_queue.empty() || (_has_next_from_bucket
                                      && _t_comparator(_next_from_bucket, _overflow_queue.top()))) {
        T ret = _next_from_bucket;
        if (_buckets_sorter[_front_bucket_idx] -> can_pull()) {
          _next_from_bucket = _buckets_sorter[_front_bucket_idx] -> pull();
        } else {
          _has_next_from_bucket = false;
        }
        return ret;
      } else {
        T ret = _overflow_queue.top();
        _overflow_queue.pop();
        return ret;
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The total number of elements distributed across the buckets and
    /// the overflow queue.
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      return _size;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Is the stream empty.
    ///
    /// If you only want to know if it is empty for the current layer, then use
    /// can_pull instead.
    ////////////////////////////////////////////////////////////////////////////
    bool empty() const
    {
      return _size == 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Internal auxiliary methods
  private:
    template<bool has_stop_label>
    void setup_next_layer(label_t stop_label)
    {
      adiar_debug(!has_stop_label || _label_comparator(front_bucket_label(), stop_label),
                 "Stop label is prior to the current front bucket");

      adiar_debug(!can_pull(),
                 "Layer is non-empty");

      adiar_debug(has_next_layer(),
                 "Has no next layer to go to");

      if constexpr (Buckets == 0) {
        while (LabelExt::label_of(_overflow_queue.top()) != _buckets_label[0]
               && (!has_stop_label || _label_comparator(_buckets_label[0], stop_label))) {
          _buckets_label[0] = label_mgr::pull();
        }
        return;
      }

      adiar_debug(_label_comparator(front_bucket_label(), back_bucket_label()),
                 "Front bucket run ahead of back bucket");

      // Sort active buckets until we find one with some content
      for (size_t b = 0;
           !_has_next_from_bucket
             && has_next_bucket()
             && (_overflow_queue.empty()
                 || !_label_comparator(LabelExt::label_of(_overflow_queue.top()), next_bucket_label()) )
             && (!has_stop_label
                 || !_label_comparator(stop_label, next_bucket_label()))
             && b <= Buckets;
                 b++) {
        setup_next_bucket();
        calc_front_bucket();
      }

      // Are we still at an empty bucket and behind the overflow queue and the stop_label?
      if (!_has_next_from_bucket && has_next_bucket() && (has_stop_label || !_overflow_queue.empty())) {
        if (!has_stop_label || !_overflow_queue.empty()) {
          label_t pq_label = LabelExt::label_of(_overflow_queue.top());
          stop_label = has_stop_label && _label_comparator(stop_label, pq_label)
            ? stop_label
            : pq_label;
        }

        if (_label_comparator(front_bucket_label(), stop_label)) {
          adiar_debug(!_overflow_queue.empty(), "Will plough through all remaining buckets");

          setup_next_bucket();
          while (has_next_bucket() && _label_comparator(front_bucket_label(), stop_label)) {
            if (label_mgr::can_pull()) {
              _buckets_label[_front_bucket_idx] = label_mgr::pull();
              _back_bucket_idx = _front_bucket_idx;
            }
            _front_bucket_idx = (_front_bucket_idx + 1) % (Buckets + 1);
          }

          calc_front_bucket();
        }
      }


      adiar_debug(!has_next_bucket() || _label_comparator(front_bucket_label(), back_bucket_label()),
                 "Inconsistency in has_next_bucket predicate");
      adiar_debug(has_next_bucket() || front_bucket_label() == back_bucket_label(),
                 "Inconsistency in has_next_bucket predicate");
    }

    bool has_next_bucket()
    {
      return _front_bucket_idx != _back_bucket_idx;
    }

    label_t next_bucket_label()
    {
      adiar_debug(has_next_bucket(), "Cannot obtain label of non-existing next bucket");

      size_t next_idx = (_front_bucket_idx + 1) % (Buckets + 1);
      label_t next_label = _buckets_label[next_idx];
      return next_label;
    }

    void setup_bucket(size_t idx, label_t label)
    {
      _buckets_label[idx] = label;

      _buckets_sorter[idx] = std::make_unique<tpie::merge_sorter<T, false, TComparator>>(_t_comparator);

      _buckets_sorter[idx] -> set_available_memory(m_single_block<T>(),
                                                   m_sort<T, Buckets>(_buckets_memory, _memory_occupied_by_meta),
                                                   m_single_block<T>());
      _buckets_sorter[idx] -> begin();
    }

    void setup_next_bucket()
    {
      adiar_debug(has_next_bucket() && _label_comparator(front_bucket_label(), back_bucket_label()),
                 "Inconsistency in has_next_bucket predicate");

      if (label_mgr::can_pull()) {
        label_t next_label = label_mgr::pull();
        setup_bucket(_front_bucket_idx, next_label);
        _back_bucket_idx = _front_bucket_idx;
      }
      _front_bucket_idx = (_front_bucket_idx + 1) % (Buckets + 1);

      adiar_debug(!has_next_bucket() || _label_comparator(front_bucket_label(), back_bucket_label()),
                 "Inconsistency in has_next_bucket predicate");
      adiar_debug(has_next_bucket() || front_bucket_label() == back_bucket_label(),
                 "Inconsistency in has_next_bucket predicate");
    }

    void calc_front_bucket()
    {
      _buckets_sorter[_front_bucket_idx] -> end();
      _buckets_sorter[_front_bucket_idx] -> calc(pq_tpie_progress_indicator);

      _has_next_from_bucket = _buckets_sorter[_front_bucket_idx] -> can_pull();
      if (_has_next_from_bucket) {
        _next_from_bucket = _buckets_sorter[_front_bucket_idx] -> pull();
      }
    }

    size_t active_buckets() const
    {
      if constexpr (Buckets == 0) {
        return 0;
      }

      return _front_bucket_idx <= _back_bucket_idx
        ? _back_bucket_idx - _front_bucket_idx
        : (Buckets + 1 - _front_bucket_idx) + _back_bucket_idx;
    }

    label_t front_bucket_label() const
    {
      return _buckets_label[_front_bucket_idx];
    }

    label_t back_bucket_label() const
    {
      return _buckets_label[_back_bucket_idx];
    }
  };

  template <typename T, typename LabelExt,
            typename TComparator = std::less<T>, typename LabelComparator = std::less<label_t>,
            size_t MetaStreams = 1u, size_t Buckets = ADIAR_PQ_BUCKETS>
  using node_priority_queue = priority_queue<node_t, 1u, T, LabelExt, TComparator, LabelComparator, MetaStreams, Buckets>;

  template <typename T, typename LabelExt,
            typename TComparator = std::less<T>, typename LabelComparator = std::less<label_t>,
            size_t MetaStreams = 1u, size_t Buckets = ADIAR_PQ_BUCKETS>
  using arc_priority_queue = priority_queue<arc_t, 2u, T, LabelExt, TComparator, LabelComparator, MetaStreams, Buckets>;
}

#endif // ADIAR_PRIORITY_QUEUE_H

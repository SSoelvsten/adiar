#ifndef COOM_PRIORITY_QUEUE_CPP
#define COOM_PRIORITY_QUEUE_CPP

#include <tpie/tpie.h>
#include <tpie/sort.h>
#include <tpie/file_stream.h>
#include <tpie/priority_queue.h>

#include "data.h"

namespace coom {
  tpie::dummy_progress_indicator pq_tpie_progress_indicator {};

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
  template <typename Comparator = std::less<label_t>, size_t MetaStreams = 1>
  class pq_label_mgr
  {
  private:
    Comparator _comparator = Comparator();

    std::vector<std::reference_wrapper<tpie::file_stream<meta_t>>> _meta_streams;
    label_t _nexts [MetaStreams];
    bool _has_nexts [MetaStreams];

  public:
    bool hook_meta_stream(tpie::file_stream<meta_t>& s)
    {
#if COOM_ASSERT
      assert (_meta_streams.size() < MetaStreams);
#endif

      size_t idx = _meta_streams.size();
      bool all_hooked = false;

      _meta_streams.push_back(std::ref(s));
      if (idx + 1 == MetaStreams)
      {
        _meta_streams.shrink_to_fit();
        all_hooked = true;
      }

      s.seek(0, tpie::file_stream_base::end);

      bool can_read = s.can_read_back();
      _has_nexts[idx] = can_read;
      if (can_read)
      {
        _nexts[idx] = s.read_back().label;
      }

      return all_hooked;
    }

    bool can_pull()
    {
#if COOM_ASSERT
      assert (_meta_streams.size() == MetaStreams);
#endif

      for (bool _has_next : _has_nexts)
      {
        if (_has_next)
        {
          return true;
        }
      }
      return false;
    }

    label_t peek()
    {
#if COOM_ASSERT
      assert (_meta_streams.size() == MetaStreams);
      assert (can_pull());
#endif
      bool has_min_label = false;
      label_t min_label = 0u;
      for (size_t idx = 0u; idx < MetaStreams; idx++)
      {
        if (_has_nexts[idx] && (!has_min_label || _comparator(_nexts[idx], min_label)))
        {
          has_min_label = true;
          min_label = _nexts[idx];
        }
      }

      return min_label;
    }

    label_t pull()
    {
#if COOM_ASSERT
      assert (_meta_streams.size() == MetaStreams);
      assert (can_pull());
#endif

      label_t min_label = peek();

      // pull from all with min_label
      for (size_t idx = 0; idx < MetaStreams; idx++) {
        if (_has_nexts[idx] && _nexts[idx] == min_label) {
          if (_meta_streams[idx].get().can_read_back()) {
            _nexts[idx] = _meta_streams[idx].get().read_back().label;
          } else {
            _has_nexts[idx] = false;
          }
        }
      }

      return min_label;
    }
  };

#ifndef COOM_PQ_BUCKETS
#define COOM_PQ_BUCKETS 1u
#endif

  //////////////////////////////////////////////////////////////////////////////
  /// A generic priority queue for OBDDs capable of improving performance by
  /// placing all pushed queue elements in buckets for the specific layer and
  /// then sorting it when one finally arrives at said layer. If no bucket
  /// exists for said request, then they will be placed in a priority queue to
  /// then be merged with the current bucket.
  ///
  /// \param T          The type of items to store.
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
  ///
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, typename LabelExt, typename TComparator = std::less<T>,
            size_t MetaStreams = 1, typename LabelComparator = std::less<label_t>,
            size_t Buckets = COOM_PQ_BUCKETS>
  class priority_queue : private LabelExt, private pq_label_mgr<LabelComparator,MetaStreams>
  {
    static_assert(0 <= Buckets && Buckets <= 4, "The number of buckets may only be in [0;4]");
    static_assert(0 < MetaStreams, "At least one meta stream should be provided");

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

    tpie::memory_size_type _buckets_memory;
    std::unique_ptr<tpie::merge_sorter<T, false, TComparator>> _buckets_sorter [Buckets + 1];

    bool _has_next_from_bucket = false;
    T _next_from_bucket;

    tpie::priority_queue<T, TComparator> _overflow_queue;

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
  public:
    priority_queue()
    {
      _buckets_memory = (tpie::get_memory_manager().available() * 3) / 4;
    }

    priority_queue(tpie::memory_size_type buckets_memory)
    {
      _buckets_memory = buckets_memory;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Public methods
  public:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Hook onto a meta stream
    ///
    /// The priority queue needs to be able to know what are the possible layers
    /// to which requests may be made. This hook_meta_stream function has to be
    /// called MetaStreams number of times before any elements are pushed.
    ////////////////////////////////////////////////////////////////////////////
    void hook_meta_stream(tpie::file_stream<meta_t>& s)
    {
#if COOM_ASSERT
      assert (_front_bucket_idx == 0);
      assert (_back_bucket_idx == 0);
#endif

      bool all_hooked = pq_label_mgr<LabelComparator, MetaStreams>::hook_meta_stream(s);
      if (all_hooked && pq_label_mgr<LabelComparator, MetaStreams>::can_pull()) {
        label_t label = pq_label_mgr<LabelComparator, MetaStreams>::pull();
        setup_bucket(_front_bucket_idx, label);

        while(_back_bucket_idx < Buckets && pq_label_mgr<LabelComparator, MetaStreams>::can_pull()) {
          label_t label = pq_label_mgr<LabelComparator, MetaStreams>::pull();
#if COOM_ASSERT
          assert (_label_comparator(_buckets_label[_back_bucket_idx], label));
#endif
          _back_bucket_idx++;
          setup_bucket(_back_bucket_idx, label);
        }

#if COOM_ASSERT
        assert (_front_bucket_idx == 0);
        assert (_back_bucket_idx <= Buckets);
#endif

        calc_front_bucket();
      }
   }

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

#if COOM_ASSERT
      assert(_label_comparator(front_bucket_label(), label));
#endif

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
      if constexpr (Buckets == 0)
      {
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
#if COOM_ASSERT
        assert (!can_pull());
#endif
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
      if constexpr (Buckets == 0)
      {
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
#if COOM_ASSERT
      assert (can_pull());
#endif

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
#if COOM_ASSERT
      assert (can_pull());
#endif

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
#if COOM_ASSERT
      assert (!has_stop_label || _label_comparator(front_bucket_label(), stop_label));
      assert (!can_pull());
      assert (has_next_layer());
#endif

      if constexpr (Buckets == 0) {
        while (LabelExt::label_of(_overflow_queue.top()) != _buckets_label[0]
               && (!has_stop_label || _label_comparator(_buckets_label[0], stop_label))) {
          _buckets_label[0] = pq_label_mgr<LabelComparator, MetaStreams>::pull();
        }
        return;
      }

#if COOM_ASSERT
      assert (_label_comparator(front_bucket_label(), back_bucket_label()));
#endif

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
#if COOM_ASSERT
          assert(!_overflow_queue.empty());
#endif

          setup_next_bucket();
          while (has_next_bucket() && _label_comparator(front_bucket_label(), stop_label)) {
            if (pq_label_mgr<LabelComparator, MetaStreams>::can_pull()) {
              _buckets_label[_front_bucket_idx] = pq_label_mgr<LabelComparator, MetaStreams>::pull();
              _back_bucket_idx = _front_bucket_idx;
            }
            _front_bucket_idx = (_front_bucket_idx + 1) % (Buckets + 1);
          }

          calc_front_bucket();
        }
      }

#if COOM_ASSERT
      assert (!has_next_bucket() || _label_comparator(front_bucket_label(), back_bucket_label()));
      assert (has_next_bucket() || front_bucket_label() == back_bucket_label());
#endif
    }

    bool has_next_bucket()
    {
      return _front_bucket_idx != _back_bucket_idx;
    }

    label_t next_bucket_label()
    {
#if COOM_ASSERT
      assert (has_next_bucket());
#endif
      size_t next_idx = (_front_bucket_idx + 1) % (Buckets + 1);
      label_t next_label = _buckets_label[next_idx];
      return next_label;
    }

    void setup_bucket(size_t idx, label_t label)
    {
      _buckets_label[idx] = label;

      /* We set up the merge_sorters such that they have a very low memory
       * footprint on Phase 1 (sending sorted blocks to disk) and Phase 3
       * (merging sorted partial results). This allows us to place a lot of
       * memory on Phase 2 (the primary sorting step), which will allow us to
       * have multiple concurrent merger_sorters, since all will mostly only use
       * the Phase 1 memory limit.
       */

      // 128 Kb * sizeof(T) + 5 MB. This is the minimum to make TPIE not cry.
      tpie::memory_size_type m_single_block = sizeof(T) * 128 * 1024 + 5 * 1024 * 1024;

      // Total amount of memory minus the blocks set aside for all buckets
      tpie::memory_size_type m_sort = _buckets_memory - m_single_block * Buckets;

#if COOM_ASSERT
      // Would we have some overflow?
      assert (m_single_block * Buckets < _buckets_memory);
#endif

      _buckets_sorter[idx] = std::make_unique<tpie::merge_sorter<T, false, TComparator>>(_t_comparator);

      _buckets_sorter[idx] -> set_available_memory(m_single_block, m_sort, m_single_block);
      _buckets_sorter[idx] -> begin();
    }

    void setup_next_bucket()
    {
#if COOM_ASSERT
      assert (has_next_bucket() && _label_comparator(front_bucket_label(), back_bucket_label()));
#endif

      if (pq_label_mgr<LabelComparator, MetaStreams>::can_pull()) {
        label_t next_label = pq_label_mgr<LabelComparator, MetaStreams>::pull();
        setup_bucket(_front_bucket_idx, next_label);
        _back_bucket_idx = _front_bucket_idx;
      }
      _front_bucket_idx = (_front_bucket_idx + 1) % (Buckets + 1);

#if COOM_ASSERT
      assert (!has_next_bucket() || _label_comparator(front_bucket_label(), back_bucket_label()));
      assert (has_next_bucket() || front_bucket_label() == back_bucket_label());
#endif
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
}

#endif // COOM_PRIORITY_QUEUE_CPP

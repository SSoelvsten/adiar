/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ADIAR_INTERNAL_LEVELIZED_PRIORITY_QUEUE_H
#define ADIAR_INTERNAL_LEVELIZED_PRIORITY_QUEUE_H

#include <tpie/tpie.h>

#include <tpie/file.h>
#include <tpie/file_stream.h>
#include <tpie/priority_queue.h>
#include <tpie/sort.h>

#include <adiar/data.h>
#include <adiar/file.h>
#include <adiar/file_stream.h>

#include <adiar/internal/sorter.h>

#include <adiar/statistics.h>

#include <adiar/bdd/bdd.h>

namespace adiar {
  //////////////////////////////////////////////////////////////////////////////
  /// We will use a stream of labels to retrieve the values from. Since this may
  /// be one or more streams (Apply, Equality, and Relational Product, and
  /// Reduce all need to consider two input streams). Luckily, the priority
  /// queue will never have to outlive its level_info stream, so we may merely just
  /// have a reference to it.
  ///
  /// What we need is a policy, that is able to merge one or more level_info streams.
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
    std::unique_ptr<level_info_stream<File_T, Files>> _meta_streams [MetaStreams];

  public:
    bool hook_meta_stream(const meta_file<File_T, Files> &f)
    {
      adiar_debug(_files_given < MetaStreams, "Given more files than was expected");

      _meta_streams[_files_given] = std::make_unique<level_info_stream<File_T, Files>>(f);

      return ++_files_given == MetaStreams;
    }

    bool can_pull()
    {
      adiar_debug(_files_given == MetaStreams,
                 "Cannot check existence of next element before being attached to all level_info streams");

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
                 "Peeking element before being attached to all expected level_info streams");
      adiar_debug(can_pull(),
                 "Cannot peek past end of all streams");

      bool has_min_label = false;
      label_t min_label = 0u;
      for (size_t idx = 0u; idx < MetaStreams; idx++) {
        if (_meta_streams[idx] -> can_pull()
            && (!has_min_label || _comparator(label_of(_meta_streams[idx] -> peek()), min_label))) {
          has_min_label = true;
          min_label = label_of(_meta_streams[idx] -> peek());
        }
      }

      return min_label;
    }

    label_t pull()
    {
      adiar_debug(_files_given == MetaStreams,
                 "Pulling element before being attached to all expected level_info streams");
      adiar_debug(can_pull(),
                 "Cannot pull past end of all streams");

      label_t min_label = peek();

      // pull from all with min_label
      for (const std::unique_ptr<level_info_stream<File_T, Files>> &level_info_stream : _meta_streams) {
        if (level_info_stream -> can_pull() && label_of(level_info_stream -> peek()) == min_label) {
          level_info_stream -> pull();
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
  /// Struct to hold statistics
  extern stats_t::priority_queue_t stats_priority_queue;

  //////////////////////////////////////////////////////////////////////////////
  /// A levelized priority queue for BDDs capable of improving performance by
  /// placing all pushed queue elements in buckets for the specific level and
  /// then sorting it when one finally arrives at said level. If no bucket
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
  /// \param MetaStreams Number of level_info information streams about the input
  ///
  /// \param LabelComparator Comparator to be used for merging multiple level_info
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
  class levelized_priority_queue
    : private LabelExt, private pq_label_mgr<File_T, Files, LabelComparator, MetaStreams>
  {
    static_assert(0 < MetaStreams, "At least one level_info stream should be provided");
    static_assert(0 < Buckets, "Bucketized levelized priority queue requires at least one bucket");

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

    tpie::memory_size_type _memory_given;
    tpie::memory_size_type _memory_occupied_by_overflow;
    tpie::memory_size_type _memory_occupied_by_meta;
    tpie::memory_size_type _memory_for_buckets;
    std::unique_ptr<external_sorter<T, TComparator>> _buckets_sorter [Buckets + 1];

    bool _has_next_from_bucket = false;
    T _next_from_bucket;

    tpie::priority_queue<T, TComparator> _overflow_queue;

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
  private:
    static tpie::memory_size_type m_overflow_queue(tpie::memory_size_type memory_given)
    {
      const tpie::memory_size_type eight_MiB = 8 * 1024;
      const tpie::memory_size_type weighted_share = memory_given / (4*(Buckets+1) + 1);

      return std::max(eight_MiB, weighted_share);
    }

    levelized_priority_queue(tpie::memory_size_type memory_given)
      : _memory_given(memory_given),
        _memory_occupied_by_overflow(m_overflow_queue(memory_given)),
        _memory_occupied_by_meta(tpie::get_memory_manager().available()),
        _overflow_queue(m_overflow_queue(memory_given))
    { }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Instantiate the priority_queue with the given amount of memory.
    ///
    /// \param memory_given    Total amount of memory the priority queue should
    ///                        take.
    ////////////////////////////////////////////////////////////////////////////
    levelized_priority_queue(const meta_file<File_T, Files> (& files) [MetaStreams],
                             tpie::memory_size_type memory_given)
      : levelized_priority_queue(memory_given)
    {
      for (const meta_file<File_T, Files> &f : files) {
        label_mgr::hook_meta_stream(f);
      }
      init_buckets();
    }

    levelized_priority_queue(const decision_diagram (& dds) [MetaStreams],
                             tpie::memory_size_type memory_given)
      : levelized_priority_queue(memory_given) {
      for (const decision_diagram& dd : dds) {
        label_mgr::hook_meta_stream(dd.file);
      }
      init_buckets();
    }

    levelized_priority_queue(const meta_file<File_T, Files> (& files) [MetaStreams])
    : levelized_priority_queue(files, tpie::get_memory_manager().available()) { }

    levelized_priority_queue(const decision_diagram (& dds) [MetaStreams])
    : levelized_priority_queue(dds, tpie::get_memory_manager().available()) { }

    ////////////////////////////////////////////////////////////////////////////
    // Private constructor methods
  private:
    void init_buckets()
    {
      // This was set in the private constructor above to be the total amount of
      // memory. This was done before the label_mgr had created all of its level_info
      // streams, so we can get how much space they already took of what we are
      // given now.
      _memory_occupied_by_meta -= tpie::get_memory_manager().available();
      _memory_for_buckets = _memory_given - _memory_occupied_by_meta - _memory_occupied_by_overflow;

      adiar_debug(_memory_occupied_by_meta + _memory_occupied_by_overflow <= _memory_given,
                  "the amount of memory used should be within the given bounds");

      if (label_mgr::can_pull()) {
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

        sort_front_bucket();
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
      label_t label = LabelExt::label_of(t);

      adiar_debug(_label_comparator(front_bucket_label(), label),
                 "Element pushed prior to currently active bucket");

      for (size_t bucket = 1; bucket <= Buckets && bucket <= active_buckets(); bucket++) {
        size_t bucket_idx = (_front_bucket_idx + bucket) % (Buckets + 1);
        if (_buckets_label[bucket_idx] == label) {
          _buckets_sorter[bucket_idx] -> push(t);
#ifdef ADIAR_STATS_EXTRA
          stats_priority_queue.push_bucket++;
#endif
          return;
        }
      }
#ifdef ADIAR_STATS_EXTRA
      stats_priority_queue.push_overflow++;
#endif
      _overflow_queue.push(t);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The label of the current level
    ////////////////////////////////////////////////////////////////////////////
    label_t current_level() const
    {
      return front_bucket_label();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Is there any non-empty level?
    ////////////////////////////////////////////////////////////////////////////
    bool has_next_level()
    {
      return _size > 0 && has_next_bucket();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set up the next nonempty level to which some requests have been
    /// pushed before the given stop_label.
    ///
    /// Sets up the next level to pull elements from.
    ///
    /// \param stop_label (optional) level that should be furthest forwarded to.
    ///
    /// If the given stop_label is larger than MAX_LABEL (i.e. it is an
    /// 'illegal' value), then it is treated as not given, i.e. it is ignored
    /// and the queue forwards to the first non-empty level. This is the default
    /// behaviour.
    ////////////////////////////////////////////////////////////////////////////
    void setup_next_level(label_t stop_label = MAX_LABEL+1)
    {
      bool has_stop_label = stop_label <= MAX_LABEL;

      adiar_debug(!has_stop_label || _label_comparator(front_bucket_label(), stop_label),
                 "Stop label should be past the current front bucket");

      adiar_debug(!can_pull(),
                 "Level should be emptied before moving on");

      adiar_debug(has_next_level(),
                 "Has no next level to go to");

      adiar_debug(_label_comparator(front_bucket_label(), back_bucket_label()),
                 "Front bucket run ahead of back bucket");

      // Sort active buckets until we find one with some content
      for (size_t b = 0;
           // At most Buckets number of iterations
           b <= Buckets
           // Is the bucket for this level empty?
           && !_has_next_from_bucket
           // Is the overflow queue without any elements for this level either?
           && (_overflow_queue.empty() || LabelExt::label_of(_overflow_queue.top()) != front_bucket_label())
           // We have no stop-label that would stop us here?
           && (!has_stop_label || stop_label != front_bucket_label())
           // Do we have more levels to go to?
           && has_next_bucket();
           b++) {
        setup_next_bucket();
        sort_front_bucket();
      }

      // Are we still at an empty bucket and behind the overflow queue and the
      // stop_label? Notice, that we have now instantiated 'Buckets' number of
      // new merge_sorters ready to place content into them. So, there is no
      // reason for us to not just keep the merge_sorter and merely fast-forward
      // on the levels.
      if (!_has_next_from_bucket && has_next_bucket()
          // && (_overflow_queue.empty() || LabelExt::label_of(_overflow_queue.top()) != front_bucket_label())
          && (!has_stop_label || stop_label != front_bucket_label())) {
        adiar_debug(!has_stop_label || _label_comparator(front_bucket_label(), stop_label),
                    "stop label should be strictly ahead of current level");
        adiar_debug(!_overflow_queue.empty(),
                    "'has_next_level()' implied non-empty queue, all buckets turned out to be empty, yet overflow queue is also.");

        label_t pq_label = LabelExt::label_of(_overflow_queue.top());
        stop_label = has_stop_label && _label_comparator(stop_label, pq_label)
          ? stop_label
          : pq_label;

        if (_label_comparator(front_bucket_label(), stop_label)) {
          setup_next_bucket();
          while (has_next_bucket() && _label_comparator(front_bucket_label(), stop_label)) {
            if (label_mgr::can_pull()) {
              _buckets_label[_front_bucket_idx] = label_mgr::pull();
              _back_bucket_idx = _front_bucket_idx;
            }
            _front_bucket_idx = (_front_bucket_idx + 1) % (Buckets + 1);
          }

          sort_front_bucket();
        }
      }

      adiar_debug(!has_next_bucket() || _label_comparator(front_bucket_label(), back_bucket_label()),
                 "Inconsistency in has_next_bucket predicate");
      adiar_debug(has_next_bucket() || front_bucket_label() == back_bucket_label(),
                 "Inconsistency in has_next_bucket predicate");
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Are there more elements on the current level?
    ////////////////////////////////////////////////////////////////////////////
    bool can_pull()
    {
      return
        _has_peeked
        // Is the current bucket non-empty?
        || _has_next_from_bucket
        // Is the priority queue non-empty and not ahead?
        || (!_overflow_queue.empty()
            && front_bucket_label() == LabelExt::label_of(_overflow_queue.top()) );
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Is the current level of elements empty?
    ////////////////////////////////////////////////////////////////////////////
    bool empty_level()
    {
      return !can_pull();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Peek the next element on the current level
    ////////////////////////////////////////////////////////////////////////////
    T peek()
    {
      adiar_debug (can_pull(), "Cannot peek on empty level/queue");

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
    /// \brief Pull the next element on the current level
    ////////////////////////////////////////////////////////////////////////////
    T pull()
    {
      adiar_debug (can_pull(), "Cannot pull on empty level/queue");

      _size--;
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
    /// If you only want to know if it is empty for the current level, then use
    /// can_pull instead.
    ////////////////////////////////////////////////////////////////////////////
    bool empty() const
    {
      return _size == 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Internal auxiliary methods
  private:
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
      _buckets_sorter[idx] = std::make_unique<external_sorter<T, TComparator>>(_memory_for_buckets, Buckets+1);
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

    void sort_front_bucket()
    {
      _buckets_sorter[_front_bucket_idx] -> sort();

      _has_next_from_bucket = _buckets_sorter[_front_bucket_idx] -> can_pull();
      if (_has_next_from_bucket) {
        _next_from_bucket = _buckets_sorter[_front_bucket_idx] -> pull();
      }
    }

    size_t active_buckets() const
    {
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
  using levelized_node_priority_queue = levelized_priority_queue<node_t, NODE_FILE_COUNT, T, LabelExt, TComparator, LabelComparator, MetaStreams, Buckets>;

  template <typename T, typename LabelExt,
            typename TComparator = std::less<T>, typename LabelComparator = std::less<label_t>,
            size_t MetaStreams = 1u, size_t Buckets = ADIAR_PQ_BUCKETS>
  using levelized_arc_priority_queue = levelized_priority_queue<arc_t, ARC_FILE_COUNT, T, LabelExt, TComparator, LabelComparator, MetaStreams, Buckets>;

  //////////////////////////////////////////////////////////////////////////////
  /// TODO: Make a levelized_priority_queue that does not have any buckets
}

#endif // ADIAR_INTERNAL_LEVELIZED_PRIORITY_QUEUE_H

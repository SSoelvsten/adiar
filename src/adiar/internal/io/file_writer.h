#ifndef ADIAR_FILE_WRITER_H
#define ADIAR_FILE_WRITER_H

#include <tpie/file_stream.h>

#include <adiar/assignment.h>
#include <adiar/internal/assert.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/data_types/ptr.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/simple_file.h>
#include <adiar/internal/io/meta_file.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/node_file.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Write-only access to a simple file including a consistency check
  ///          on the given input.
  ///
  /// \details The consistency check verifies, whether something is allowed to
  ///          come after something else. In all our current use-cases, the
  ///          check induces a total ordering.
  //////////////////////////////////////////////////////////////////////////////
  template<typename T>
  struct no_ordering
  {
    bool operator()(const T&, const T&) const
    { return true; }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Write-only access to a simple file including a consistency check
  ///          on the given input.
  ///
  /// \param T    Type of the file's content
  ///
  /// \param Comp The logic to check for consistency (usually a comparator)
  ///
  /// \details The consistency check verifies, whether something is allowed to
  ///          come after something else. In all our current use-cases, the
  ///          check induces a total ordering.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, typename Comp = no_ordering<T>>
  class simple_file_writer
  {
  public:
    static size_t memory_usage()
    {
      return tpie::file_stream<T>::memory_usage();
    }

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// The file stream includes a shared pointer to hook into the reference
    /// counting and garbage collection of the file.
    ////////////////////////////////////////////////////////////////////////////
    shared_ptr<file<T>> _file_ptr;

    tpie::file_stream<T> _stream;

    Comp _comp = Comp();

    bool _has_latest = false;
    T _latest;

  public:
    simple_file_writer() { }
    simple_file_writer(const simple_file<T> &f) { attach(f); }

    ~simple_file_writer() { detach(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a file
    ////////////////////////////////////////////////////////////////////////////
    void attach(const simple_file<T> &f)
    {
      if (attached()) { detach(); }
      _file_ptr = f._file_ptr;

      adiar_assert(!(_file_ptr -> is_read_only()), "Cannot attach a writer onto a read-only file");

      _stream.open(_file_ptr -> _tpie_file, ADIAR_WRITE_ACCESS);
      _stream.seek(0, tpie::file_stream_base::end);

      // Set up tracker of latest element added
      _has_latest = _stream.can_read_back();
      if (_has_latest) {
        _latest = _stream.read_back();
        _stream.read();
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the writer currently is attached to any file.
    ////////////////////////////////////////////////////////////////////////////
    bool attached() const
    {
      return _stream.is_open();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detach from a file (if need be).
    ////////////////////////////////////////////////////////////////////////////
    void detach()
    {
      adiar_assert(!(_file_ptr && _file_ptr -> is_read_only()),
                   "Stream was detached after someone started reading from it");
      _stream.close();
      // if (_file_ptr) { _file_ptr.reset(); }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write an element to the file (without any checks).
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const T &t)
    {
      _stream.write(t);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Sort the current content of file based on the given comparator.
    ///          Default sorting predicate is the validity checking logic.
    ////////////////////////////////////////////////////////////////////////////
    template<typename sorting_pred_t>
    void sort(sorting_pred_t pred = sorting_pred_t())
    {
      adiar_debug(attached(), "Cannot sort no content");

      tpie::progress_indicator_null pi;
      tpie::sort(_stream, pred, pi);

      // Reset the _latest value.
      _stream.seek(0, tpie::file_stream_base::end);
      if (_has_latest) {
        _latest = _stream.read_back();
        _stream.read();
      }
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Sort the current content of file based on the validity checker.
    ////////////////////////////////////////////////////////////////////////////
    void sort()
    {
      sort<Comp>(_comp);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write the element to the file (with consistency checks).
    ////////////////////////////////////////////////////////////////////////////
    void push(const T &t)
    {
      adiar_assert(attached(), "file_writer is not yet attached to any file");

      // Check is sorted input
      adiar_assert(!_has_latest || _comp(_latest, t),
                   "The given element must be provided in order");

      unsafe_push(t);
    }

    simple_file_writer<T,Comp>& operator<< (const T& t)
    {
      this -> push(t);
      return *this;
    }
  };

  typedef simple_file_writer<assignment_t, std::less<assignment_t>> assignment_writer;
  typedef simple_file_writer<ptr_uint64::label_t, no_ordering<ptr_uint64::label_t>> label_writer;


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Writer to a set of file(s) with 'meta' information.
  ///
  /// \sa node_writer arc_writer
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class meta_file_writer
  {
  public:
    static size_t memory_usage()
    {
      return FILE_CONSTANTS<T>::files * tpie::file_stream<T>::memory_usage()
        + tpie::file_stream<level_info>::memory_usage();
    }

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// The file stream includes a shared pointer to hook into the reference
    /// counting and garbage collection of the file.
    ////////////////////////////////////////////////////////////////////////////
    shared_ptr<__meta_file<T>> _file_ptr;

    tpie::file_stream<level_info_t> _meta_stream;
    tpie::file_stream<T> _streams [FILE_CONSTANTS<T>::files];

  public:
    meta_file_writer() { }

    meta_file_writer(const meta_file<T> &f)
    {
      attach(f);
    }

    ~meta_file_writer() { detach(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a file
    ////////////////////////////////////////////////////////////////////////////
    void attach(const meta_file<T> &f)
    {
      if (attached()) { detach(); }
      _file_ptr = f._file_ptr;

      adiar_assert(!(_file_ptr -> _level_info_file.is_read_only()),
                   "Cannot attach a writer onto a read-only meta file");

      _meta_stream.open(_file_ptr->_level_info_file._tpie_file);
      _meta_stream.seek(0, tpie::file_stream_base::end);

      for (size_t idx = 0; idx < FILE_CONSTANTS<T>::files; idx++) {
        adiar_assert(!(_file_ptr->_files[idx].is_read_only()),
                     "Cannot attach a writer onto a read-only content file");

        _streams[idx].open(_file_ptr->_files[idx]._tpie_file);
        _streams[idx].seek(0, tpie::file_stream_base::end);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the writer currently is attached.
    ////////////////////////////////////////////////////////////////////////////
    bool attached() const
    {
      return _meta_stream.is_open();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detach from a file (if need be)
    ////////////////////////////////////////////////////////////////////////////
    void detach()
    {
      if (!attached()) { return; }

      _meta_stream.close();
      for (size_t idx = 0; idx < FILE_CONSTANTS<T>::files; idx++) {
        _streams[idx].close();
      }

      _file_ptr.reset();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Write directly (and solely) to the level_info file without any
    ///          checks for consistency.
    ///
    /// \param m Level information to push
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const level_info_t &m)
    {
      _meta_stream.write(m);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief     Write directly (and solely) to some file without any checks
    ///            for consistency.
    ///
    /// \param t   Element to push
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const T &t, size_t idx = 0)
    {
      adiar_debug(idx < FILE_CONSTANTS<T>::files, "Invalid index");
      _streams[idx].write(t);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether anything has been pushed to any of the underlying files.
    ////////////////////////////////////////////////////////////////////////////
    bool has_pushed()
    {
      for (size_t idx = 0; idx < FILE_CONSTANTS<T>::files; idx++) {
        if (_streams[idx].size() > 0) {
          return true;
        }
      }

      return _meta_stream.size() > 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the underlying file is empty.
    ////////////////////////////////////////////////////////////////////////////
    bool empty()
    {
      return !has_pushed();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of elements pushed to the underlying files (excluding the
    ///        level info file).
    ////////////////////////////////////////////////////////////////////////////
    size_t size()
    {
      size_t acc = 0u;
      for (size_t idx = 0; idx < FILE_CONSTANTS<T>::files; idx++) {
        acc += _streams[idx].size();
      }
      return acc;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of elements in the level info file.
    ////////////////////////////////////////////////////////////////////////////
    size_t levels()
    {
      return _meta_stream.size();
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Writer for nodes, hiding derivation of all meta information and
  /// applying sanity checks on the validity of the input.
  ///
  /// \sa node_file
  //////////////////////////////////////////////////////////////////////////////
  class node_writer: public meta_file_writer<node>
  {
  private:
    ////////////////////////////////////////////////////////////////////////////
    /// Construct a dummy node, that is invalid within a decision diagram and
    /// hence easy to recognise.
    ////////////////////////////////////////////////////////////////////////////
    node dummy()
    {
      // Notice, this goes around any of the consistency checks of 'node'!
      return node(node::uid_t(0, 0),
                            node::ptr_t::NIL(),
                            node::ptr_t::NIL());
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Buffer of latest pushed element, such that one can compare with it.
    ////////////////////////////////////////////////////////////////////////////
    node _latest_node = dummy();

    ////////////////////////////////////////////////////////////////////////////
    /// Canonicity flag.
    ////////////////////////////////////////////////////////////////////////////
    bool _canonical = true;

    ////////////////////////////////////////////////////////////////////////////
    /// Number of nodes pushed to the current level.
    ////////////////////////////////////////////////////////////////////////////
    size_t _level_size = 0u;

    ////////////////////////////////////////////////////////////////////////////
    /// Variables for 1-level cut
    /// We will count the following globally
    /// - The number of arcs pushed at the very bottom
    /// - The number of (long) arcs that cross at least one pushed level
    ///
    /// While for each level we can safely count
    /// - The number of (short) arcs from a single level to the next
    ////////////////////////////////////////////////////////////////////////////
    size_t _terminals_at_bottom[2] = { 0u, 0u };

    cut_size_t _max_1level_short_internal = 0u;
    cut_size_t _curr_1level_short_internal = 0u;

    node::ptr_t _long_internal_ptr = node::ptr_t::NIL();
    cut_size_t _number_of_long_internal_arcs = 0u;

  public:
    node_writer() : meta_file_writer() { }
    node_writer(const node_file &nf)
      : meta_file_writer(nf),
        _canonical(!meta_file_writer::has_pushed()|| nf->canonical)
    { }

    ~node_writer() { detach(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief   Write the next node to the file (and check consistency).
    ///
    /// \details Writes the given node to the end of the file and also writes to
    ///          the level_info file if necessary. The given node must have
    ///          valid children (not checked), no duplicate nodes created (not
    ///          properly checked), and must be topologically prior to any nodes
    ///          already written to the file (checked).
    ////////////////////////////////////////////////////////////////////////////
    void push(const node &n)
    {
      adiar_assert(attached(), "file_writer is not yet attached to any file");

      if (_latest_node == dummy()) { // First node pushed
        _canonical = n.is_terminal() || n.id() == node::MAX_ID;
      } else { // Check validity of input based on prior written node
        adiar_debug(!_latest_node.is_terminal(),
                     "Cannot push a node after having pushed a terminal");
        adiar_debug(!n.is_terminal(),
                     "Cannot push a terminal into non-empty file");

        // Check it is canonically sorted
        if (_canonical) {
          if (_latest_node.label() == n.label()) {
            bool id_diff = n.uid().id() == _latest_node.id() - 1u;
            bool children_ordered = n.high() < _latest_node.high()
              || (n.high() == _latest_node.high() && n.low() < _latest_node.low());

            _canonical = id_diff && children_ordered;
          } else {
            bool id_reset = n.id() == node::MAX_ID;
            _canonical = id_reset;
          }
        }

        // Check if this is the first node of a new level
        if (n.label() != _latest_node.label()) {
          // Update level information with the level just finished
          meta_file_writer::unsafe_push(create_level_info(_latest_node.label(),
                                                                    _level_size));
          _level_size = 0u;

          // Update 1-level cut information
          _max_1level_short_internal = std::max(_max_1level_short_internal,
                                                _curr_1level_short_internal);

          _curr_1level_short_internal = 0u;
          _long_internal_ptr = node::uid_t(_latest_node.label(), node::MAX_ID);
        }
      }

      // 1-level cut
      const bool is_pushing_to_bottom = _long_internal_ptr == node::ptr_t::NIL();
      if (is_pushing_to_bottom && !n.is_terminal()) {
        _terminals_at_bottom[n.low().value()]++;
        _terminals_at_bottom[n.high().value()]++;
      }

      if (n.low().is_node()) {
        if (n.low() > _long_internal_ptr) { _number_of_long_internal_arcs++; }
        else { _curr_1level_short_internal++; }
      }

      if (n.high().is_node()) {
        if (n.high() > _long_internal_ptr) { _number_of_long_internal_arcs++; }
        else { _curr_1level_short_internal++; }
      }

      // Update terminal counters
      if (n.low().is_terminal()) { _file_ptr->number_of_terminals[n.low().value()]++; }
      if (n.high().is_terminal()) { _file_ptr->number_of_terminals[n.high().value()]++; }
      if (n.uid().is_terminal()) { _file_ptr->number_of_terminals[n.uid().value()]++; }

      // Write node to file
      _latest_node = n;
      _level_size++;

      meta_file_writer::unsafe_push(n, 0);
    }

    node_writer& operator<< (const node& n)
    {
      this -> push(n);
      return *this;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to level information file without any checks.
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const level_info_t &m)
    { meta_file_writer::unsafe_push(m); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to the underlying node file without any checks.
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const node &n)
    {
      meta_file_writer::unsafe_push(n, 0);

      if (n.low().is_terminal()) { _file_ptr->number_of_terminals[n.low().value()]++; }
      if (n.high().is_terminal()) { _file_ptr->number_of_terminals[n.high().value()]++; }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Overwrite the number of false and true arcs.
    ////////////////////////////////////////////////////////////////////////////
    void set_number_of_terminals(size_t number_of_false, size_t number_of_true)
    {
      _file_ptr->number_of_terminals[false] = number_of_false;
      _file_ptr->number_of_terminals[true]  = number_of_true;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Increase the 1-level cut size to the maximum of the current or
    ///        the given cuts.
    ////////////////////////////////////////////////////////////////////////////
    void inc_1level_cut(const cuts_t &o)
    {
      inc_cut(_file_ptr->max_1level_cut, o);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Increase the 2-level cut size to the maximum of the current or
    ///        the given cuts.
    ////////////////////////////////////////////////////////////////////////////
    void inc_2level_cut(const cuts_t &o)
    {
      inc_cut(_file_ptr->max_2level_cut, o);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a file
    ////////////////////////////////////////////////////////////////////////////
    void attach(const node_file &f) {
      meta_file_writer::attach(f);

      //Reset all meta-data
      _latest_node = node(node::uid_t(0, 0),
                                    node::ptr_t::NIL(),
                                    node::ptr_t::NIL());

      _canonical = true;

      _level_size = 0u;

      _terminals_at_bottom[0] = 0u;
      _terminals_at_bottom[1] = 0u;

      _max_1level_short_internal = 0u;
      _curr_1level_short_internal = 0u;

      _long_internal_ptr = node::ptr_t::NIL();
      _number_of_long_internal_arcs = 0u;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the writer currently is attached.
    ////////////////////////////////////////////////////////////////////////////
    bool attached() const
    { return meta_file_writer::attached(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detach from a file (if need be)
    ////////////////////////////////////////////////////////////////////////////
    void detach()
    {
      if (!attached()) { return; }

      _file_ptr -> canonical = _canonical;

      // Has '.push' been used?
      if (_latest_node != dummy()) {
        // Output level information of the final level
        if (!_latest_node.is_terminal()) {
          meta_file_writer::unsafe_push(create_level_info(_latest_node.label(),
                                                                    _level_size));
        }

        _level_size = 0u; // TODO: move to attach...?

        // 1-level cut
        _max_1level_short_internal = std::max(_max_1level_short_internal,
                                              _curr_1level_short_internal);

        const cut_size_t max_1level_internal_cut =
          _max_1level_short_internal + _number_of_long_internal_arcs;

        _file_ptr->max_1level_cut[cut_type::INTERNAL] = max_1level_internal_cut;

        const size_t terminals_above_bottom[2] = {
          _file_ptr->number_of_terminals[false] - _terminals_at_bottom[false],
          _file_ptr->number_of_terminals[true]  - _terminals_at_bottom[true]
        };

        _file_ptr->max_1level_cut[cut_type::INTERNAL_FALSE] =
          std::max(max_1level_internal_cut + terminals_above_bottom[false],
                   _file_ptr->number_of_terminals[false]);

        _file_ptr->max_1level_cut[cut_type::INTERNAL_TRUE] =
          std::max(max_1level_internal_cut + terminals_above_bottom[true],
                   _file_ptr->number_of_terminals[true]);

        _file_ptr->max_1level_cut[cut_type::ALL] =
          std::max(max_1level_internal_cut + terminals_above_bottom[false] + terminals_above_bottom[true],
                   _file_ptr->number_of_terminals[false] + _file_ptr->number_of_terminals[true]);
      }

      // Run final i-level cut computations
      fixup_ilevel_cuts();

      meta_file_writer::detach();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether anything has been pushed to any of the underlying files.
    ////////////////////////////////////////////////////////////////////////////
    bool has_pushed()
    { return meta_file_writer::has_pushed(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the underlying file is empty.
    ////////////////////////////////////////////////////////////////////////////
    bool empty()
    { return meta_file_writer::empty(); }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Helper function to bound derived 1-level and 2-level cuts with
    ///        their trivial upper bounds (assuming nothing is pushed later).
    ////////////////////////////////////////////////////////////////////////////
    void fixup_ilevel_cuts()
    {
      const size_t number_of_nodes = meta_file_writer::size();
      const size_t number_of_false = _file_ptr->number_of_terminals[false];
      const size_t number_of_true = _file_ptr->number_of_terminals[true];

      // -----------------------------------------------------------------------
      // Upper bound for any directed cut based on number of internal nodes.
      const cut_size_t max_cut = number_of_nodes < MAX_CUT
        ? number_of_nodes + 1
        : MAX_CUT;

      // -----------------------------------------------------------------------
      // Upper bound on just 'all arcs'. This is better than 'max_cut' above, if
      // there are 'number_of_nodes' or more arcs to terminals.
      const bool noa_overflow_safe = number_of_nodes <= MAX_CUT / 2u;
      const size_t number_of_arcs = 2u * number_of_nodes;

      const cuts_t all_arcs_cut = {
        noa_overflow_safe ? number_of_arcs - number_of_false - number_of_true : MAX_CUT,
        noa_overflow_safe ? number_of_arcs - number_of_true                   : MAX_CUT,
        noa_overflow_safe ? number_of_arcs - number_of_false                  : MAX_CUT,
        noa_overflow_safe ? number_of_arcs                                    : MAX_CUT
      };

      // -----------------------------------------------------------------------
      // Maximum 1-level cuts
      const bool is_terminal = number_of_false + number_of_true == 1;

      if (is_terminal) {
        _file_ptr->max_1level_cut[cut_type::INTERNAL]       = 0u;
        _file_ptr->max_1level_cut[cut_type::INTERNAL_FALSE] = number_of_false;
        _file_ptr->max_1level_cut[cut_type::INTERNAL_TRUE]  = number_of_true;
        _file_ptr->max_1level_cut[cut_type::ALL]            = 1u;
      } else {
        for(size_t ct = 0u; ct < CUT_TYPES; ct++) {
          // Use smallest sound upper bound. Since it is not a terminal, then there
          // must be at least one in-going arc to the root.
          _file_ptr->max_1level_cut[ct] = std::max(1lu, std::min({
                _file_ptr->max_1level_cut[ct],
                max_cut,
                all_arcs_cut[ct]
              }));
        }
      }

      // -----------------------------------------------------------------------
      // Maximum 2-level cut
      const size_t number_of_levels = meta_file_writer::levels();

      if (is_terminal || number_of_nodes == number_of_levels) {
        for(size_t ct = 0u; ct < CUT_TYPES; ct++) {
          _file_ptr->max_2level_cut[ct] = _file_ptr->max_1level_cut[ct];
        }
      } else { // General case
        for(size_t ct = 0u; ct < CUT_TYPES; ct++) {
          // Upper bound based on 1-level cut
          const cut_size_t ub_from_1level_cut =
            _file_ptr->max_1level_cut[ct] < MAX_CUT / 3u
            ? ((_file_ptr->max_1level_cut[cut_type::INTERNAL] * 3u) / 2u
               + (_file_ptr->max_1level_cut[ct] - _file_ptr->max_1level_cut[cut_type::INTERNAL]))
            : MAX_CUT;

          // Use smallest sound upper bound.
          _file_ptr->max_2level_cut[ct] = std::min({
              _file_ptr->max_2level_cut[ct],
              ub_from_1level_cut,
              max_cut,
              all_arcs_cut[ct]
            });
        }
      }
    }

    void inc_cut(cuts_t &c, const cuts_t &o)
    {
      for(size_t ct = 0u; ct < CUT_TYPES; ct++) {
        c[ct] = std::max(c[ct], o[ct]);
      }
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Writer for a set of arcs.
  ///
  /// \sa arc_file
  //////////////////////////////////////////////////////////////////////////////
  class arc_writer: public meta_file_writer<arc>
  {
  private:
    bool __has_latest_terminal = false;
    arc __latest_terminal;

  public:
    arc_writer() { }
    arc_writer(const arc_file &af) {
      attach(af);
    }

    ~arc_writer()
    {
      detach();
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to level information file without any checks.
    //////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const level_info_t &m)
    { meta_file_writer::unsafe_push(m); }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Write an arc to the relevant underlying file without any checks
    ///
    /// \sa unsafe_push_node unsafe_push_terminal
    //////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const arc &a)
    {
      adiar_debug(!a.target().is_nil(), "Should not push an arc to NIL.");
      if (a.target().is_node()) {
        unsafe_push_node(a);
      } else { // a.target().is_terminal()
        unsafe_push_terminal(a);
      }
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Write an internal arc to its file, i.e. where the target is a node.
    //////////////////////////////////////////////////////////////////////////////
    void unsafe_push_node(const arc &a)
    {
      adiar_precondition(a.target().is_node());
      meta_file_writer::unsafe_push(a, 0);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Write a terminal arc to its file, i.e. where the target is a terminal.
    //////////////////////////////////////////////////////////////////////////////
    void unsafe_push_terminal(const arc &a)
    {
      adiar_precondition(a.target().is_terminal());

      if (!__has_latest_terminal || a.source() > __latest_terminal.source()) { // in-order
        __has_latest_terminal = true;
        __latest_terminal = a;
        meta_file_writer::unsafe_push(a, 1);
      } else { // out-of-order
        meta_file_writer::unsafe_push(a, 2);
      }

      _file_ptr->number_of_terminals[a.target().value()]++;
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a file
    //////////////////////////////////////////////////////////////////////////////
    void attach(const arc_file &af) {
      meta_file_writer::attach(af);
      adiar_debug(meta_file_writer::empty(), "Attached to non-empty arc_file");
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the writer currently is attached.
    //////////////////////////////////////////////////////////////////////////////
    bool attached() const
    { return meta_file_writer::attached(); }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Sort the out-of-order terminal arcs and then detach from a file (if
    ///        need be).
    //////////////////////////////////////////////////////////////////////////////
    void detach() {
      if (attached() && _streams[2].size() > 0) {
        tpie::progress_indicator_null pi;
        tpie::sort(_streams[2], arc_source_lt(), pi);
      }

      return meta_file_writer::detach();
    }
  };
}

#endif // ADIAR_FILE_WRITER_H

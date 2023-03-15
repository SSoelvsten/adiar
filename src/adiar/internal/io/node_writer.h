#ifndef ADIAR_INTERNAL_IO_NODE_WRITER_H
#define ADIAR_INTERNAL_IO_NODE_WRITER_H

#include <adiar/internal/cut.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/io/levelized_file_writer.h>
#include <adiar/internal/io/node_file.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Writer for nodes, hiding derivation of all meta information and
  /// applying sanity checks on the validity of the input.
  ///
  /// \sa shared_levelized_file<node>
  //////////////////////////////////////////////////////////////////////////////
  class node_writer: public levelized_file_writer<node>
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
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct unattached to any levelized node file.
    ////////////////////////////////////////////////////////////////////////////
    node_writer(): levelized_file_writer<node>()
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a levelized node file.
    ////////////////////////////////////////////////////////////////////////////
    node_writer(levelized_file<node> &nf)
      : levelized_file_writer<node>(nf)
      , _canonical(!levelized_file_writer::has_pushed() || nf.canonical)
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a shared levelized node file.
    ////////////////////////////////////////////////////////////////////////////
    node_writer(adiar::shared_ptr<levelized_file<node>> nf)
      : levelized_file_writer<node>(nf)
      , _canonical(!levelized_file_writer::has_pushed()|| nf->canonical)
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches and cleans up from the levelized file (if need be).
    ////////////////////////////////////////////////////////////////////////////
    ~node_writer()
    { detach(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a file
    ////////////////////////////////////////////////////////////////////////////
    void attach(adiar::shared_ptr<levelized_file<node>> &f) {
      levelized_file_writer::attach(f);

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
    { return levelized_file_writer::attached(); }

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
          unsafe_push(level_info(_latest_node.label(), _level_size));
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

      levelized_file_writer::detach();
    }

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
        if (n.is_terminal()) {
          _file_ptr->number_of_terminals[n.uid().value()]++;
        }
      } else { // Check validity of input based on prior written node
        adiar_debug(!_latest_node.is_terminal(),
                     "Cannot push a node after having pushed a terminal");
        adiar_debug(!n.is_terminal(),
                     "Cannot push a terminal into non-empty file");

        // Check it is canonically sorted
        if (_canonical) {
          if (_latest_node.label() == n.label()) {
            const bool id_diff = n.uid().id() == _latest_node.id() - 1u;
            const bool children_ordered = n.high() < _latest_node.high()
              || (n.high() == _latest_node.high() && n.low() < _latest_node.low());

            _canonical = id_diff && children_ordered;
          } else {
            const bool id_reset = n.id() == node::MAX_ID;
            _canonical = id_reset;
          }
        }

        // Check if this is the first node of a new level
        if (n.label() != _latest_node.label()) {
          // Update level information with the level just finished
          unsafe_push(level_info(_latest_node.label(), _level_size));
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

      // Write node to file
      _latest_node = n;
      _level_size++;

      unsafe_push(n);
    }

    node_writer& operator<< (const node& n)
    {
      this->push(n);
      return *this;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to level information file without any checks.
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const level_info &m)
    {
      _file_ptr->width = std::max<size_t>(_file_ptr->width, m.width());

      levelized_file_writer::push(m);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to the underlying node file without any checks.
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_push(const node &n)
    {
      if (n.low().is_terminal())  { _file_ptr->number_of_terminals[n.low().value()]++; }
      if (n.high().is_terminal()) { _file_ptr->number_of_terminals[n.high().value()]++; }

      levelized_file_writer::template push<0>(n);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Increase the 1-level cut size to the maximum of the current or
    ///        the given cuts.
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_inc_1level_cut(const cuts_t &o)
    {
      inc_cut(_file_ptr->max_1level_cut, o);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Increase the 2-level cut size to the maximum of the current or
    ///        the given cuts.
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_inc_2level_cut(const cuts_t &o)
    {
      inc_cut(_file_ptr->max_2level_cut, o);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Overwrite the number of false and true arcs.
    ////////////////////////////////////////////////////////////////////////////
    void unsafe_set_number_of_terminals(size_t number_of_false, size_t number_of_true)
    {
      _file_ptr->number_of_terminals[false] = number_of_false;
      _file_ptr->number_of_terminals[true]  = number_of_true;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether anything has been pushed to any of the underlying files.
    ////////////////////////////////////////////////////////////////////////////
    bool has_pushed()
    { return levelized_file_writer::has_pushed(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the underlying file is empty.
    ////////////////////////////////////////////////////////////////////////////
    bool empty()
    { return levelized_file_writer::empty(); }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Helper function to bound derived 1-level and 2-level cuts with
    ///        their trivial upper bounds (assuming nothing is pushed later).
    ////////////////////////////////////////////////////////////////////////////
    void fixup_ilevel_cuts()
    {
      const size_t number_of_nodes = levelized_file_writer::size();
      const size_t number_of_false = _file_ptr->number_of_terminals[false];
      const size_t number_of_true  = _file_ptr->number_of_terminals[true];

      // -----------------------------------------------------------------------
      // Upper bound for any directed cut based on number of internal nodes.
      const cut_size_t max_cut = number_of_nodes < MAX_CUT // overflow?
        ? number_of_nodes + 1
        : MAX_CUT;

      // -----------------------------------------------------------------------
      // Upper bound on just 'all arcs'. This is better than 'max_cut' above, if
      // there are 'number_of_nodes' or more arcs to terminals.
      const bool noa_overflow_safe = number_of_nodes <= MAX_CUT / 2u;
      const size_t number_of_arcs = 2u * number_of_nodes;

      const cuts_t all_arcs_cut = {{
          noa_overflow_safe ? number_of_arcs - number_of_false - number_of_true : MAX_CUT,
          noa_overflow_safe ? number_of_arcs - number_of_true                   : MAX_CUT,
          noa_overflow_safe ? number_of_arcs - number_of_false                  : MAX_CUT,
          noa_overflow_safe ? number_of_arcs                                    : MAX_CUT
        }};

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
          _file_ptr->max_1level_cut[ct] = std::max<cut_size_t>(1lu, std::min<cut_size_t>({
                _file_ptr->max_1level_cut[ct],
                max_cut,
                all_arcs_cut[ct]
              }));
        }
      }

      // -----------------------------------------------------------------------
      // Maximum 2-level cut
      const size_t number_of_levels = levelized_file_writer::levels();

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
}

#endif // ADIAR_INTERNAL_IO_NODE_WRITER_H

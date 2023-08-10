#ifndef ADIAR_INTERNAL_IO_NODE_RANDOM_ACCESS_H
#define ADIAR_INTERNAL_IO_NODE_RANDOM_ACCESS_H

#include <adiar/internal/data_types/node.h>

#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/levelized_file_stream.h>

namespace adiar::internal
{
  // TODO: Generalize parts of 'node_random_access' to reuse it with levelized
  // files with other types of content. Yet, what use-case do we have for this?

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Random-access to the contents of a levelized file of node.
  //////////////////////////////////////////////////////////////////////////////
  template<typename node_t = node, bool reverse = false>
  class node_random_access
  {
    static_assert(!reverse, "Reversed logic is not implemented.");

    using uid_t   = typename node_t::uid_t;
    using label_t = typename node_t::label_t;
    using id_t    = typename node_t::id_t;

  public:
    static size_t memory_usage(tpie::memory_size_type max_width)
    {
      return node_stream<reverse>::memory_usage()
           + tpie::array<node_t>::memory_usage(max_width);
    }

    static size_t memory_usage(const dd &diagram)
    {
      return node_stream<reverse>::memory_usage()
        + tpie::array<node_t>::memory_usage(diagram->width);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Value to mark there is no current level.
    ////////////////////////////////////////////////////////////////////////////
    static constexpr label_t NO_LEVEL = -1;

    static_assert(uid_t::MAX_LABEL < NO_LEVEL,
                  "'NO_LEVEL' should be an invalid label value");

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief File stream to obtain the contents of each level
    ////////////////////////////////////////////////////////////////////////////
    node_stream<reverse> _ns;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Maximum width of the contents of 'lfs'. This is the maximum
    /// number of elements needed to be placed within.
    ////////////////////////////////////////////////////////////////////////////
    const id_t _max_width;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Buffer with all elements of the current level.
    ////////////////////////////////////////////////////////////////////////////
    tpie::array<node_t> _level_buffer;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Buffer with all elements of the current level.
    ////////////////////////////////////////////////////////////////////////////
    label_t _curr_level = NO_LEVEL;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Width of the current level.
    ////////////////////////////////////////////////////////////////////////////
    id_t _curr_width = 0;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Root of the diagram.
    ////////////////////////////////////////////////////////////////////////////
    uid_t _root;

    // TODO: Add canonicity flag to discern whether to compute the index (as
    //       now) or to use binary search.

  public:
    ////////////////////////////////////////////////////////////////////////////
    // node_random_access()
    // { }
    //
    // TODO: Add 'attach(...)', 'attached()' 'detach()' working multi-usage.

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a levelized file of nodes.
    ///
    /// \pre The given levelized file is canonical.
    ////////////////////////////////////////////////////////////////////////////
    node_random_access(const levelized_file<node_t> &f,
                       const bool negate = false)
      : _ns(f, negate), _max_width(f.width), _level_buffer(f.width)
    {
      adiar_assert(f.canonical);
      init();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a shared levelized file of nodes.
    ///
    /// \pre The given shared levelized file is canonical.
    ////////////////////////////////////////////////////////////////////////////
    node_random_access(const shared_ptr<levelized_file<node_t>> &f,
                       const bool negate = false)
      : _ns(f, negate), _max_width(f->width), _level_buffer(f->width)
    {
      adiar_assert(f->canonical);
      init();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a decision diagram.
    ///
    /// \pre The given decision diagram is canonical.
    ////////////////////////////////////////////////////////////////////////////
    node_random_access(const dd &diagram)
      : _ns(diagram), _max_width(diagram->width), _level_buffer(diagram->width)
    {
      adiar_assert(diagram->canonical);
      init();
    }

  private:
    void init()
    {
      adiar_assert(_ns.can_pull(), "given file should be non-empty");

      // Skip the terminal node for terminal only BDDs.
      _root = _ns.peek().uid();
      if (_root.is_terminal()) {
        _ns.pull();
      }
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Return root of the diagram.
    ////////////////////////////////////////////////////////////////////////////
    uid_t root() const
    { return _root; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there are any more levels.
    ////////////////////////////////////////////////////////////////////////////
    bool has_next_level() const
    { return _ns.can_pull(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The immediate next available non-empty level.
    ///
    /// \pre `has_next_level() == true`
    ////////////////////////////////////////////////////////////////////////////
    label_t next_level()
    { return _ns.peek().uid().label(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Sets up the random access buffer for the specified level.
    ///
    /// \param level The next level to provide random access to. If the requested
    ///              level does not exist, then the buffer will be empty.
    ///
    /// \pre `has_current_level() == false` or `current_level() < level`
    ////////////////////////////////////////////////////////////////////////////
    void setup_next_level(const label_t level)
    {
      adiar_assert(!has_current_level() || current_level() < level);

      // Set to new level and mark the entire buffer's content garbage.
      _curr_level = level;
      _curr_width = 0;

      // Stop early when going "beyond" the available levels
      if (!has_next_level()) { return; }

      // Skip all levels not of interest
      while (_ns.can_pull() && _ns.peek().uid().label() < level) {
        _ns.pull();
      }

      // Copy over all elements from the requested level
      while (_ns.can_pull() && _ns.peek().uid().label() == level) {
        _level_buffer[_curr_width++] = _ns.pull();
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Sets up the random access buffer for the immediate next available
    ///        non-empty level.
    ////////////////////////////////////////////////////////////////////////////
    void setup_next_level()
    { setup_next_level(next_level()); }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there is any current level to access elements from.
    ////////////////////////////////////////////////////////////////////////////
    bool has_current_level() const
    { return _curr_level != NO_LEVEL; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The label of the current level.
    ////////////////////////////////////////////////////////////////////////////
    label_t current_level() const
    { return _curr_level; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The width of the current level, i.e. the number of elements one
    ///        can access to.
    ////////////////////////////////////////////////////////////////////////////
    label_t current_width() const
    { return _curr_width; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the current level is empty.
    ////////////////////////////////////////////////////////////////////////////
    bool empty_level() const
    { return _curr_width == 0u; }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the element of the current level at the given index.
    ///
    /// \pre `idx < current_width()`
    ////////////////////////////////////////////////////////////////////////////
    const node_t& at(id_t idx) const
    {
      adiar_assert(idx < current_width());
      return _level_buffer[idx];
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the node with the given uid.
    ////////////////////////////////////////////////////////////////////////////
    const node_t& at(uid_t u) const
    {
      adiar_assert(u.label() == current_level());

      // adiar_assert(... < current_width()); is in 'return at(...)'
      return at(current_width() - ((uid_t::MAX_ID + 1u) - u.id()));
    }
  };
}

#endif // ADIAR_INTERNAL_IO_NODE_RANDOM_ACCESS_H

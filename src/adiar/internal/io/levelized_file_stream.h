#ifndef ADIAR_INTERNAL_IO_LEVELIZED_FILE_STREAM
#define ADIAR_INTERNAL_IO_LEVELIZED_FILE_STREAM

#include <adiar/internal/assert.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/levelized_file.h>

// TODO: move?
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/io/node_file.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief         File stream of levelized files.
  ///
  /// \param T       The type of the file(s)'s elements
  ///
  /// \param reverse Whether the reading direction should be reversed
  ///
  /// \remark Since the content of all levelized files are generated in reverse
  ///         of the desired reading order, then 'reversing' the reversed input
  ///         is equivalent to not reversing the underlying stream. Hence, we do
  ///         hide a negation of the \em reverse parameter.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_t, bool reverse = false>
  class levelized_file_stream
  {
  public:
    static constexpr size_t streams = FILE_CONSTANTS<elem_t>::files;
    static_assert(0 < streams, "There must be at least a single file to attach to.");

    static size_t memory_usage()
    {
      return streams * file_stream<elem_t, reverse>::memory_usage();
    }

  private:
    file_stream<elem_t, reverse> _streams[streams];

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create unattached to any file.
    ////////////////////////////////////////////////////////////////////////////
    levelized_file_stream()
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create attached to a levelized file.
    ////////////////////////////////////////////////////////////////////////////
    levelized_file_stream(const levelized_file<elem_t> &lf,
                          bool negate = false)
    { attach(lf, negate); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create attached to a shared levelized file.
    ////////////////////////////////////////////////////////////////////////////
    levelized_file_stream(const shared_ptr<levelized_file<elem_t>> &lf,
                          bool negate = false)
    { attach(lf, negate); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches and cleans up when destructed.
    ////////////////////////////////////////////////////////////////////////////
    ~levelized_file_stream() = default; // <-- detach is within 'file_stream'.

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a levelized file.
    ///
    /// \pre No `levelized_file_writer` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    void attach(const levelized_file<elem_t> &f,
                bool negate = false)
    {
      if (!f.exists()) f.touch();

      for (size_t s_idx = 0; s_idx < streams; s_idx++)
        _streams[s_idx].attach(f._files[s_idx], nullptr, negate);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a shared levelized file.
    ///
    /// \pre No `levelized_file_writer` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    void attach(const shared_ptr<levelized_file<elem_t>> &f,
                bool negate = false)
    {
      if (!f->exists()) f->touch();

      for (size_t s_idx = 0; s_idx < streams; s_idx++)
        _streams[s_idx].attach(f->_files[s_idx], f, negate);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether this stream is attached to a levelized file.
    ////////////////////////////////////////////////////////////////////////////
    bool attached() const
    {
      const bool res = _streams[0].attached();
#ifndef NDEBUG
      for (size_t s_idx = 1; s_idx < streams; s_idx++) {
        adiar_debug(_streams[s_idx].attached() == res,
                    "Attachment ought to be synchronised.");
      }
#endif
      return res;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches the stream from a levelized file.
    ////////////////////////////////////////////////////////////////////////////
    void detach()
    {
      for (size_t s_idx = 0; s_idx < streams; s_idx++)
        _streams[s_idx].detach();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Reset the read heads back to the beginning (relatively to the
    ///        reading direction).
    ////////////////////////////////////////////////////////////////////////////
    void reset()
    {
      for (size_t s_idx = 0; s_idx < streams; s_idx++)
        _streams[s_idx].reset();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the sub-stream contains more elements.
    ////////////////////////////////////////////////////////////////////////////
    template<size_t s_idx>
    bool can_pull() const
    {
      static_assert(s_idx < streams, "Sub-stream index must be within [0; streams).");
      return _streams[s_idx].can_pull();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain next element from a sub-stream (and move its read head).
    ///
    /// \pre `can_pull<s_idx>() == true`.
    ////////////////////////////////////////////////////////////////////////////
    template<size_t s_idx>
    elem_t pull()
    {
      static_assert(s_idx < streams, "Sub-stream index must be within [0; streams)");
      return _streams[s_idx].pull();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next element from the specified sub-stream (but do not
    ///        move the read head).
    ///
    /// \pre `can_pull<s_idx>() == true`.
    ////////////////////////////////////////////////////////////////////////////
    template<size_t s_idx>
    elem_t peek()
    {
      static_assert(s_idx < streams, "Sub-stream index must be within [0; streams)");
      return _streams[s_idx].peek();
    }
  };


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Stream to access per-level meta information.
  //////////////////////////////////////////////////////////////////////////////
  template <bool REVERSE = false>
  class level_info_stream : public file_stream<level_info_t, !REVERSE>
  {
    using parent_t = file_stream<level_info_t, !REVERSE>;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct unattached to a levelized file.
    ////////////////////////////////////////////////////////////////////////////
    level_info_stream() = default;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a file<level_info>.
    ////////////////////////////////////////////////////////////////////////////
    level_info_stream(const file<level_info_t> &f)
    { parent_t::attach(f); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a file<level_info>.
    ////////////////////////////////////////////////////////////////////////////
    level_info_stream(const adiar::shared_ptr<file<level_info_t>> &f)
    { parent_t::attach(f); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a levelized file.
    ////////////////////////////////////////////////////////////////////////////
    template<typename elem_t>
    level_info_stream(const levelized_file<elem_t, false> &lf)
    { attach(lf); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a shared levelized file.
    ////////////////////////////////////////////////////////////////////////////
    template<typename elem_t>
    level_info_stream(const adiar::shared_ptr<levelized_file<elem_t, false>> &lf)
    { attach(lf); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a (reduced) decision diagram.
    ////////////////////////////////////////////////////////////////////////////
    level_info_stream(const dd &diagram)
    { attach(diagram); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a (possibly unreduced) decision diagram.
    ///
    /// \warning Intended for unit testing only!
    ////////////////////////////////////////////////////////////////////////////
    level_info_stream(const __dd &diagram)
    { attach(diagram); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches and cleans up when destructed.
    ////////////////////////////////////////////////////////////////////////////
    ~level_info_stream() = default; // <-- detach in '~file<level_info_t>()'

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a levelized file.
    ////////////////////////////////////////////////////////////////////////////
    template<typename elem_t>
    void attach(const levelized_file<elem_t, false> &lf)
    {
      if (!lf.exists()) lf.touch();
      parent_t::attach(lf._level_info_file, nullptr, false);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a shared levelized file.
    ////////////////////////////////////////////////////////////////////////////
    template<typename elem_t>
    void attach(const adiar::shared_ptr<levelized_file<elem_t, false>> &lf)
    {
      if (!lf->exists()) lf->touch();
      parent_t::attach(lf->_level_info_file, lf, false);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a (reduced) decision diagram.
    ////////////////////////////////////////////////////////////////////////////
    void attach(const dd &diagram)
    { attach<node>(diagram.file); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a (possibly unreduced) decision diagram.
    ///
    /// \warning Intended for unit testing only!
    ////////////////////////////////////////////////////////////////////////////
    void attach(const __dd &diagram)
    {
      // TODO: switch order to favour levelized arc files
      if (diagram.has<__dd::shared_nodes_t>()) {
        attach<node>(diagram.get<__dd::shared_nodes_t>());
      } else if (diagram.has<__dd::shared_arcs_t>()) {
        attach<arc>(diagram.get<__dd::shared_arcs_t>());
      } else {
        // We should never be in the case of hooking into a 'no_file'. That type
        // should only be used internally within an algorithm and never escape
        // into its output.
        adiar_unreachable();
      }
    }
  };
}

#endif // ADIAR_INTERNAL_IO_LEVELIZED_FILE_STREAM

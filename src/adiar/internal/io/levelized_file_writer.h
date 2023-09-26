#ifndef ADIAR_INTERNAL_IO_LEVELIZED_FILE_WRITER_H
#define ADIAR_INTERNAL_IO_LEVELIZED_FILE_WRITER_H

#include <adiar/internal/assert.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/file_writer.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/node_file.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Writer to a set of file(s) with 'meta' information.
  ///
  /// \sa node_writer arc_writer
  //////////////////////////////////////////////////////////////////////////////
  template <typename value_t>
  class levelized_file_writer
  {
  public:
    using value_type = value_t;

    static size_t memory_usage()
    {
      return file_traits<value_type>::files * file_writer<value_type>::memory_usage()
           + 1u * file_writer<level_info>::memory_usage();
    }

  protected:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Pointer to update the meta information.
    ////////////////////////////////////////////////////////////////////////////
    adiar::shared_ptr<levelized_file<value_type>> _file_ptr;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Writers for the file with level information.
    ////////////////////////////////////////////////////////////////////////////
    file_writer<level_info> _level_writer;

    static constexpr size_t elem_writers = file_traits<value_type>::files;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Writers for each of the files with 'value_type'.
    ////////////////////////////////////////////////////////////////////////////
    file_writer<value_type> _elem_writers [elem_writers];

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct unattached to any levelized file.
    ////////////////////////////////////////////////////////////////////////////
    levelized_file_writer() { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a given levelized file.
    ///
    /// \pre No file stream or other writer is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    levelized_file_writer(levelized_file<value_type> &f)
    { attach(f); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a given shared levelized file.
    ///
    /// \pre No file stream or other writer is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    levelized_file_writer(adiar::shared_ptr<levelized_file<value_type>> f)
    { attach(f); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches and cleans up when destructed.
    ////////////////////////////////////////////////////////////////////////////
    ~levelized_file_writer()
    { } // <-- detach within `~file_writer<...>()`

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a file.
    ///
    /// \warning Since ownership is \em not shared with this writer, you have to
    ///          ensure, that the file in question is not destructed before
    ///          `.detach()` is called.
    ////////////////////////////////////////////////////////////////////////////
    void attach(levelized_file<value_type> &f)
    {
      if (attached()) { detach(); }

      // The stack variable is made accessible in '_file_ptr', but it should not
      // be garbage collected. Hence, we provide a do-nothing deleter to the
      // 'std::shared_ptr' directly.
      _file_ptr = std::shared_ptr<levelized_file<value_type>>(&f, [](void *) {});

      _level_writer.attach(f._level_info_file, nullptr);
      for (size_t s_idx = 0; s_idx < file_traits<value_type>::files; s_idx++)
        _elem_writers[s_idx].attach(f._files[s_idx], nullptr);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a shared file.
    ////////////////////////////////////////////////////////////////////////////
    void attach(adiar::shared_ptr<levelized_file<value_type>> f)
    {
      if (attached()) { detach(); }

      _file_ptr = f;

      _level_writer.attach(f->_level_info_file, f);
      for (size_t s_idx = 0; s_idx < file_traits<value_type>::files; s_idx++)
        _elem_writers[s_idx].attach(f->_files[s_idx], f);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the writer currently is attached to a levelized file.
    ////////////////////////////////////////////////////////////////////////////
    bool attached() const
    {
      const bool res = _level_writer.attached();
#ifndef NDEBUG
      for (size_t s_idx = 0; s_idx < elem_writers; s_idx++) {
        adiar_assert(_elem_writers[s_idx].attached() == res,
                     "Attachment ought to be synchronised.");
      }
#endif
      return res;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detach from a levelized file (if need be)
    ////////////////////////////////////////////////////////////////////////////
    void detach()
    {
      _level_writer.detach();
      for (size_t s_idx = 0; s_idx < elem_writers; s_idx++)
        _elem_writers[s_idx].detach();
      _file_ptr.reset();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to the level_info file.
    ///
    /// \param li Level information to push.
    ///
    /// \pre `attached() == true`.
    ////////////////////////////////////////////////////////////////////////////
    void push(const level_info &li)
    {
      _level_writer.push(li);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write directly to the level_info file.
    ///
    /// \param li Level information to push.
    ///
    /// \pre `attached() == true`.
    ////////////////////////////////////////////////////////////////////////////
    levelized_file_writer<value_type>& operator<< (const level_info& li)
    {
      this->push(li);
      return *this;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Write to one of the files.
    ///
    /// \tparam s_idx File index to push to.
    /// \param  e     Element to push.
    ////////////////////////////////////////////////////////////////////////////
    template<size_t s_idx>
    void push(const value_type &e)
    {
      static_assert(s_idx < elem_writers,
                    "Sub-stream index must be within [0; elem_writers).");

      _elem_writers[s_idx].push(e);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of elements written to the underlying files (excluding the
    ///        level info file).
    ////////////////////////////////////////////////////////////////////////////
    size_t size(const size_t s_idx) const
    {
      adiar_assert(s_idx < elem_writers,
                   "Sub-stream index must be within [0; elem_writers).");
      return _elem_writers[s_idx].size();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of elements written to the underlying files (excluding the
    ///        level info file).
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    {
      size_t acc = 0u;
      for (size_t s_idx = 0; s_idx < elem_writers; s_idx++) {
        acc += size(s_idx);
      }
      return acc;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of elements in the level info file.
    ////////////////////////////////////////////////////////////////////////////
    size_t levels() const
    { return _level_writer.size(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether anything has been pushed to any of the underlying files.
    ////////////////////////////////////////////////////////////////////////////
    bool has_pushed() const
    {
      for (size_t s_idx = 0; s_idx < elem_writers; s_idx++) {
        if (size(s_idx) > 0) { return true; }
      }
      return levels() > 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the underlying file is empty.
    ////////////////////////////////////////////////////////////////////////////
    bool empty() const
    { return !has_pushed(); }
  };
}

#endif // ADIAR_INTERNAL_IO_LEVELIZED_FILE_WRITER_H

#ifndef ADIAR_INTERNAL_IO_FILE_WRITER_H
#define ADIAR_INTERNAL_IO_FILE_WRITER_H

#include <tpie/file_stream.h>
#include <tpie/sort.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/data_types/level_info.h>
#include <adiar/internal/data_types/ptr.h>
#include <adiar/internal/io/file.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Write-only access to a simple file including a consistency check
  ///          on the given input.
  ///
  /// \tparam  value_t Type of the file's content.
  ///
  /// \details The consistency check verifies, whether something is allowed to
  ///          come after something else. In all our current use-cases, the
  ///          check induces a total ordering.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class file_writer
  {
  public:
    using value_type = T;

  public:
    static size_t memory_usage()
    {
      return tpie::file_stream<value_type>::memory_usage();
    }

  private:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief If attached to a shared file then hook into the reference
    ///        counting such that the file is not garbage collected while we
    ///        write to it.
    ////////////////////////////////////////////////////////////////////////////
    shared_ptr<void> _file_ptr;

    ////////////////////////////////////////////////////////////////////////////
    /// \brief TPIE's file stream object to write elements with.
    ////////////////////////////////////////////////////////////////////////////
    tpie::file_stream<value_type> _stream;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct unattached to any file.
    ////////////////////////////////////////////////////////////////////////////
    file_writer()
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a given `file<value_type>`.
    ///
    /// \pre No `file_stream` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    file_writer(file<value_type> &f)
    { attach(f); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a given shared `file<value_type>`.
    ///
    /// \pre No `file_stream` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    file_writer(adiar::shared_ptr<file<value_type>> &f)
    { attach(f); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches and cleans up when destructed.
    ////////////////////////////////////////////////////////////////////////////
    ~file_writer()
    { detach(); }

  protected:
    ////////////////////////////////////////////////////////////////////////////
    void attach(file<value_type> &f, adiar::shared_ptr<void> p)
    {
      if (f.is_persistent())
        throw runtime_error("Cannot attach writer to a persisted file");

      if (attached()) { detach(); }
      _file_ptr = p;

      _stream.open(f._tpie_file, file<value_type>::write_access);
      _stream.seek(0, tpie::file_stream_base::end);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Befriend the few places that need direct access to the above 'attach'.
    template <typename tparam__elem_t>
    friend class levelized_file_writer;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a file.
    ///
    /// \pre No `file_stream` is currently attached to this file.
    ///
    /// \warning Since ownership is \em not shared with this writer, you have to
    ///          ensure, that the file in question is not destructed before
    ///          `.detach()` is called.
    ////////////////////////////////////////////////////////////////////////////
    void attach(file<value_type> &f)
    { attach(f, nullptr); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a shared file.
    ///
    /// \pre No `file_stream` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    void attach(adiar::shared_ptr<file<value_type>> f)
    { attach(*f, f); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the writer currently is attached to any file.
    ////////////////////////////////////////////////////////////////////////////
    bool attached() const
    { return _stream.is_open(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detach from a file (if need be).
    ////////////////////////////////////////////////////////////////////////////
    void detach()
    {
      _stream.close();
      if (_file_ptr) { _file_ptr.reset(); }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Push an element to the end of the file.
    ///
    /// \pre `attached() == true`.
    ////////////////////////////////////////////////////////////////////////////
    void push(const value_type &e)
    { _stream.write(e); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Push an element to the end of the file.
    ///
    /// \pre `attached() == true`.
    ////////////////////////////////////////////////////////////////////////////
    file_writer<value_type>& operator<< (const value_type& e)
    {
      this->push(e);
      return *this;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether anything has been pushed the file.
    ///
    /// \pre `attached() == true`.
    ////////////////////////////////////////////////////////////////////////////
    bool has_pushed() const
    { return _stream.size() > 0; }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether the underlying file is empty.
    ///
    /// \pre `attached() == true`.
    ////////////////////////////////////////////////////////////////////////////
    bool empty() const
    { return !has_pushed(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Number of elements written to this file.
    ///
    /// \pre `attached() == true`.
    ////////////////////////////////////////////////////////////////////////////
    size_t size() const
    { return _stream.size(); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Sort the content of the attached file.
    ///
    /// \pre `attached() == true`.
    ////////////////////////////////////////////////////////////////////////////
    template <typename pred_t = std::less<value_type>>
    void sort(const pred_t pred = pred_t())
    {
      if (empty()) return;

      tpie::progress_indicator_null pi;
      tpie::sort(_stream, pred, pi);
    }
  };

  // TODO: remove...
  using label_writer = file_writer<ptr_uint64::label_type> ;
}

#endif // ADIAR_INTERNAL_IO_FILE_WRITER_H

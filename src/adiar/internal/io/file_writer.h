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

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief   Write-only access to a simple file including a consistency check
  ///          on the given input.
  ///
  /// \param elem_t Type of the file's content.
  ///
  /// \details The consistency check verifies, whether something is allowed to
  ///          come after something else. In all our current use-cases, the
  ///          check induces a total ordering.
  //////////////////////////////////////////////////////////////////////////////
  template <typename elem_type>
  class file_writer
  {
  public:
    typedef elem_type elem_t;

  public:
    static size_t memory_usage()
    {
      return tpie::file_stream<elem_t>::memory_usage();
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
    tpie::file_stream<elem_t> _stream;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct unattached to any file.
    ////////////////////////////////////////////////////////////////////////////
    file_writer()
    { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a given `file<elem_t>`.
    ///
    /// \pre No `file_stream` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    file_writer(file<elem_t> &f)
    { attach(f); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a given shared `file<elem_t>`.
    ///
    /// \pre No `file_stream` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    file_writer(const shared_ptr<file<elem_t>> &f)
    { attach(f); }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Detaches and cleans up when destructed.
    ////////////////////////////////////////////////////////////////////////////
    ~file_writer()
    { detach(); }

  protected:
    ////////////////////////////////////////////////////////////////////////////
    void attach(file<elem_t> &f, const adiar::shared_ptr<void> &p)
    {
      if (attached()) { detach(); }
      _file_ptr = p;

      _stream.open(f._tpie_file, file<elem_t>::write_access);
      _stream.seek(0, tpie::file_stream_base::end);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a file.
    ///
    /// \pre No `file_stream` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    void attach(file<elem_t> &f)
    {
      attach(f, nullptr);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach to a shared file.
    ///
    /// \pre No `file_stream` is currently attached to this file.
    ////////////////////////////////////////////////////////////////////////////
    void attach(const adiar::shared_ptr<file<elem_t>> &f)
    {
      attach(*f, f);
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
      _stream.close();
      if (_file_ptr) { _file_ptr.reset(); }
    }

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Push an element to the end of the file.
    ////////////////////////////////////////////////////////////////////////////
    void push(const elem_t &e)
    {
      _stream.write(e);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Push an element to the end of the file.
    ////////////////////////////////////////////////////////////////////////////
    file_writer<elem_t>& operator<< (const elem_t& e)
    {
      this->push(e);
      return *this;
    }
  };

  // TODO: remove...
  typedef file_writer<assignment_t> assignment_writer;
  typedef file_writer<ptr_uint64::label_t> label_writer;
}

#endif // ADIAR_FILE_WRITER_H

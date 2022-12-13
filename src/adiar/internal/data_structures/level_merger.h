#ifndef ADIAR_INTERNAL_DATA_STRUCTURES_LEVEL_MERGER_H
#define ADIAR_INTERNAL_DATA_STRUCTURES_LEVEL_MERGER_H

#include <adiar/internal/dd.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/levelized_file_stream.h>
#include <adiar/internal/io/shared_file_ptr.h>
#include <adiar/internal/util.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Defines at compile time the type of the file stream to use for
  ///        reading the levels from some file(s).
  //////////////////////////////////////////////////////////////////////////////
  template <typename file_t>
  struct level_stream_t
  {
    typedef level_info_stream<> stream_t;
  };

  template <>
  struct level_stream_t<file<ptr_uint64::label_t>>
  {
    typedef file_stream<ptr_uint64::label_t> stream_t;
  };

  template <>
  struct level_stream_t<shared_file<ptr_uint64::label_t>>
  {
    typedef file_stream<ptr_uint64::label_t> stream_t;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Merges the labels from one or more files.
  ///
  /// \param file_t Type of the files to read from
  ///
  /// \param comp_t Comparator with which to merge the labels
  ///
  /// \param FILES  Number of files to read from
  //////////////////////////////////////////////////////////////////////////////
  template <typename file_t, typename comp_t, size_t FILES>
  class level_merger
  {
    static_assert(0 < FILES, "At least one file should be merged");

    typedef typename level_stream_t<file_t>::stream_t stream_t;

  public:
    static size_t memory_usage()
    {
      return FILES * stream_t::memory_usage();
    }

    typedef ptr_uint64::label_t level_t;

  private:
    comp_t _comparator = comp_t();

    unique_ptr<stream_t> _level_streams [FILES];

  public:
    void hook(const file_t (&fs) [FILES])
    {
      for (size_t idx = 0u; idx < FILES; idx++) {
        _level_streams[idx] = adiar::make_unique<stream_t>(fs[idx]);
      }
    }

    void hook(const dd (&dds) [FILES])
    {
      for (size_t idx = 0u; idx < FILES; idx++) {
        _level_streams[idx] = adiar::make_unique<stream_t>(dds[idx].file);
      }
    }

    bool can_pull()
    {
      for (size_t idx = 0u; idx < FILES; idx++) {
        if (_level_streams[idx] -> can_pull()) {
          return true;
        }
      }
      return false;
    }

    level_t peek()
    {
      adiar_debug(can_pull(),
                  "Cannot peek past end of all streams");

      bool has_min_level = false;
      level_t min_level = 0u;
      for (size_t idx = 0u; idx < FILES; idx++) {
        if (_level_streams[idx] -> can_pull()
            && (!has_min_level || _comparator(__level_of<>(_level_streams[idx] -> peek()), min_level))) {
          has_min_level = true;
          min_level = __level_of<>(_level_streams[idx] -> peek());
        }
      }

      return min_level;
    }

    level_t pull()
    {
      adiar_debug(can_pull(),
                  "Cannot pull past end of all streams");

      level_t min_level = peek();

      // pull from all with min_level
      for (const unique_ptr<stream_t> &level_info_stream : _level_streams) {
        if (level_info_stream->can_pull() && __level_of<>(level_info_stream->peek()) == min_level) {
          level_info_stream->pull();
        }
      }
      return min_level;
    }
  };
}

#endif //  ADIAR_INTERNAL_DATA_STRUCTURES_LEVEL_MERGER_H

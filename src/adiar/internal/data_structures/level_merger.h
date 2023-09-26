#ifndef ADIAR_INTERNAL_DATA_STRUCTURES_LEVEL_MERGER_H
#define ADIAR_INTERNAL_DATA_STRUCTURES_LEVEL_MERGER_H

#include <adiar/internal/assert.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/memory.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/file_stream.h>
#include <adiar/internal/io/levelized_file_stream.h>
#include <adiar/internal/io/shared_file_ptr.h>
#include <adiar/internal/util.h>

namespace adiar::internal
{
  // TODO (code clarity):
  //   Add to 'file_t' an enum with 'Ascending'/'Descending' to then derive the
  //   comparator in conjunction with 'reverse'.

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Merges the levels from one or more files.
  ///
  /// \tparam file_t Type of the files to read from.
  ///
  /// \tparam comp_t Comparator with which to merge the levels.
  ///
  /// \tparam file_count  Number of files to read from.
  //////////////////////////////////////////////////////////////////////////////
  template<typename file_t, typename comp_t, size_t file_count, bool reverse = false>
  class level_merger
  {
    static_assert(0 < file_count, "At least one file should be merged");

    using stream_t = typename level_stream_t<file_t>::template stream_t<reverse>;

  public:
    static size_t memory_usage()
    {
      return file_count * stream_t::memory_usage();
    }

    using level_type = ptr_uint64::label_type;

  private:
    comp_t _comparator = comp_t();

    unique_ptr<stream_t> _level_streams[file_count];

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach onto the given list of files.
    ////////////////////////////////////////////////////////////////////////////
    void hook(const file_t (&fs) [file_count])
    {
      for (size_t idx = 0u; idx < file_count; idx++) {
        _level_streams[idx] = adiar::make_unique<stream_t>(fs[idx]);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach onto the given list of decision diagrams.
    ////////////////////////////////////////////////////////////////////////////
    void hook(const dd (&dds) [file_count])
    {
      for (size_t idx = 0u; idx < file_count; idx++) {
        _level_streams[idx] = adiar::make_unique<stream_t>(dds[idx].file);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Attach onto the given list of (unreduced) decision diagrams.
    ////////////////////////////////////////////////////////////////////////////
    void hook(const __dd (&dds) [file_count])
    {
      for (size_t idx = 0u; idx < file_count; idx++) {
        _level_streams[idx] = adiar::make_unique<stream_t>(dds[idx]);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Whether there are more levels to fetch.
    ////////////////////////////////////////////////////////////////////////////
    bool can_pull()
    {
      for (size_t idx = 0u; idx < file_count; idx++) {
        if (_level_streams[idx]->can_pull()) {
          return true;
        }
      }
      return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next level.
    ///
    /// \pre `can_pull() == true`
    ////////////////////////////////////////////////////////////////////////////
    level_type peek()
    {
      adiar_assert(can_pull(), "Cannot peek past end of all streams");

      bool has_min_level = false;
      level_type min_level = 0u;
      for (size_t idx = 0u; idx < file_count; idx++) {
        if (_level_streams[idx]->can_pull()
            && (!has_min_level || _comparator(level_of(_level_streams[idx]->peek()), min_level))) {
          has_min_level = true;
          min_level = level_of(_level_streams[idx]->peek());
        }
      }

      return min_level;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the next level and go to the next.
    ///
    /// \pre `can_pull() == true`
    ////////////////////////////////////////////////////////////////////////////
    level_type pull()
    {
      adiar_assert(can_pull(), "Cannot pull past end of all streams");

      level_type min_level = peek();

      // pull from all with min_level
      for (const unique_ptr<stream_t> &level_info_stream : _level_streams) {
        if (level_info_stream->can_pull() && level_of(level_info_stream->peek()) == min_level) {
          level_info_stream->pull();
        }
      }
      return min_level;
    }
  };
}

#endif //  ADIAR_INTERNAL_DATA_STRUCTURES_LEVEL_MERGER_H

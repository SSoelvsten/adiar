#ifndef ADIAR_INTERNAL_UTIL_H
#define ADIAR_INTERNAL_UTIL_H

#include <type_traits>

#include <adiar/functional.h>

#include <adiar/internal/assert.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/convert.h>
#include <adiar/internal/io/file.h>
#include <adiar/internal/io/shared_file_ptr.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/levelized_file_stream.h>
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_writer.h>
#include <adiar/internal/io/node_stream.h>

namespace adiar::internal
{
  ////////////////////////////////////////////////////////////////////////////
  /// \brief Template to hide how to obtain the level from a data type.
  ////////////////////////////////////////////////////////////////////////////
  template<typename T>
  inline ptr_uint64::label_type
  level_of(const T& t)
  {
    if constexpr (std::is_integral<T>::value) {
      return t;
    } else {
      return t.level();
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Defines at compile time the type of the file stream to use for
  ///        reading the levels from some file(s).
  //////////////////////////////////////////////////////////////////////////////
  template<typename file_t>
  struct level_stream_t
  {
    template<bool reverse = false>
    using stream_t = level_info_stream<reverse>;
  };

  template<>
  struct level_stream_t<file<ptr_uint64::label_type>>
  {
    template<bool reverse = false>
    using stream_t = file_stream<ptr_uint64::label_type, reverse>;
  };

  template<>
  struct level_stream_t<shared_file<ptr_uint64::label_type>>
  {
    template<bool reverse = false>
    using stream_t = file_stream<ptr_uint64::label_type, reverse>;
  };

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain whether the levels in two files are disjoint.
  ////////////////////////////////////////////////////////////////////////////
  // TODO: Move to dd_func?
  template<typename in1_t, typename in2_t>
  bool
  disjoint_levels(const in1_t &in1, const in2_t &in2)
  {
    using stream1_t = typename level_stream_t<in1_t>::template stream_t<false>;
    stream1_t s1(in1);

    using stream2_t = typename level_stream_t<in2_t>::template stream_t<false>;
    stream2_t s2(in2);

    while(s1.can_pull() && s2.can_pull()) {
      if (level_of(s1.peek()) == level_of(s2.peek())) {
        return false;
      } else if (level_of(s1.peek()) < level_of(s2.peek())) {
        s1.pull();
      } else {
        s2.pull();
      }
    }
    return true;
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a certain level exists in a file.
  ////////////////////////////////////////////////////////////////////////////
  // TODO: Move to dd_func?
  template<typename dd_t>
  bool
  has_level(const dd_t &in, const typename dd_t::label_type l)
  {
    level_info_stream<> in_meta(in);
    while(in_meta.can_pull()) {
      level_info m = in_meta.pull();

      // Are we already past where it should be?
      if (l < m.label())  { return false; }

      // Did we find it?
      if (m.label() == l) { return true; }
    }
    return false;
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the semi-transposition of a decision diagram.
  ///
  /// \details Creates an `arc_file` of the given diagram. This file is on a
  ///          valid form to be an input to the bottom-up `reduce` algorithm:
  ///          internal arcs are transposed, i.e. sorted on their target in
  ///          ascending order, while arcs to terminals are left untransposed,
  ///          i.e. sorted on their source).
  ///
  /// \pre `!is_terminal(dd)`
  ///
  /// \see reduce
  ////////////////////////////////////////////////////////////////////////////
  template <typename dd_t>
  shared_levelized_file<arc>
  transpose(const dd_t &dd)
  {
    adiar_assert(!dd->is_terminal());

    shared_levelized_file<arc> af;

    // Create the contents of 'af'
    { arc_writer aw(af);
      { // Split every node into their arcs.
        node_stream ns(dd);
        while (ns.can_pull()) {
          const typename dd_t::node_type n = ns.pull();

          // TODO (non-binary nodes):
          aw << low_arc_of(n);
          aw << high_arc_of(n);
        }
      }
      { // Copy over meta information
        af->max_1level_cut = dd->max_1level_cut[cut::Internal];

        level_info_stream<> lis(dd);
        while (lis.can_pull()) {
          aw << lis.pull();
        }
      }
    }

    // Sort internal arcs by their target
    af->sort<arc_target_lt>(file_traits<arc>::idx__internal);
    return af;
  }
}

#endif // ADIAR_INTERNAL_UTIL_H

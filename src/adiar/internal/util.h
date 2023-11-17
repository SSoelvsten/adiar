#ifndef ADIAR_INTERNAL_UTIL_H
#define ADIAR_INTERNAL_UTIL_H

#include <type_traits>

#include <adiar/functional.h>
#include <adiar/types.h>

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
  template<typename File>
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
  template<typename A, typename B>
  bool
  disjoint_levels(const A &a, const B &b)
  {
    using stream1_t = typename level_stream_t<A>::template stream_t<false>;
    stream1_t sa(a);

    using stream2_t = typename level_stream_t<B>::template stream_t<false>;
    stream2_t sb(b);

    while(sa.can_pull() && sb.can_pull()) {
      if (level_of(sa.peek()) == level_of(sb.peek())) {
        return false;
      } else if (level_of(sa.peek()) < level_of(sb.peek())) {
        sa.pull();
      } else {
        sb.pull();
      }
    }
    return true;
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Whether a certain level exists in a file.
  ////////////////////////////////////////////////////////////////////////////
  // TODO: Move to dd_func?
  template<typename DD>
  bool
  has_level(const DD &d, const typename DD::label_type x)
  {
    level_info_stream<> in_meta(d);
    while(in_meta.can_pull()) {
      level_info m = in_meta.pull();

      // Are we already past where it should be?
      if (x < m.label())  { return false; }

      // Did we find it?
      if (m.label() == x) { return true; }
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
  template <typename DD>
  shared_levelized_file<arc>
  transpose(const DD &d)
  {
    adiar_assert(!d->is_terminal());

    shared_levelized_file<arc> af;

    // Create the contents of 'af'
    { arc_writer aw(af);
      { // Split every node into their arcs.
        node_stream ns(d);
        while (ns.can_pull()) {
          const typename DD::node_type n = ns.pull();

          // TODO (non-binary nodes):
          aw << low_arc_of(n);
          aw << high_arc_of(n);
        }
      }
      { // Copy over meta information
        af->max_1level_cut = d->max_1level_cut[cut::Internal];

        level_info_stream<> lis(d);
        while (lis.can_pull()) {
          aw << lis.pull();
        }
      }
    }

    // Sort internal arcs by their target
    af->sort<arc_target_lt>(file_traits<arc>::idx__internal);
    return af;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Wrap a `generator<int>` into a `generator<pair<label_type, bool>>`
  ///        where the boolean value is true if the value is negative.
  //////////////////////////////////////////////////////////////////////////////
  template<typename DdPolicy, typename Generator>
  generator<pair<typename DdPolicy::label_type, bool>>
  wrap_signed_generator(const Generator &g)
  {
    using label_type  = typename DdPolicy::label_type;

    return [&g]() -> optional<pair<label_type, bool>> {
      const typename Generator::result_type next_opt = g();

      if (next_opt) {
        const pair<label_type, bool> ret_value = make_pair(std::abs(next_opt.value()),
                                                           next_opt.value() < 0);
        return make_optional(ret_value);
      }
      return make_optional<pair<label_type, bool>>();
    };
  }
}

#endif // ADIAR_INTERNAL_UTIL_H

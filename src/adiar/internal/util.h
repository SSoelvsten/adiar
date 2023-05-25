#ifndef ADIAR_INTERNAL_UTIL_H
#define ADIAR_INTERNAL_UTIL_H

#include <adiar/internal/assert.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/data_types/arc.h>
#include <adiar/internal/data_types/convert.h>
#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/levelized_file_stream.h>

// TODO: remove these imports
#include <adiar/internal/io/arc_file.h>
#include <adiar/internal/io/arc_writer.h>
#include <adiar/internal/io/node_file.h>
#include <adiar/internal/io/node_stream.h>

namespace adiar::internal
{
  ////////////////////////////////////////////////////////////////////////////
  /// \brief Template to hide how to extra the level from a certain data type.
  ////////////////////////////////////////////////////////////////////////////
  template<typename T>
  inline ptr_uint64::label_t
  __level_of(const T& t)
  { return t.level(); }

  template<>
  inline ptr_uint64::label_t
  __level_of(const ptr_uint64::label_t& l)
  { return l; }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain whether the levels in two files are disjoint.
  ////////////////////////////////////////////////////////////////////////////
  template<typename in1_t = dd, typename stream1_t = level_info_stream<>,
           typename in2_t = dd, typename stream2_t = level_info_stream<>>
  bool
  disjoint_levels(const in1_t &in1, const in2_t &in2)
  {
    stream1_t s1(in1);
    stream2_t s2(in2);

    while(s1.can_pull() && s2.can_pull()) {
      if (__level_of<>(s1.peek()) == __level_of<>(s2.peek())) {
        return false;
      } else if (__level_of<>(s1.peek()) < __level_of<>(s2.peek())) {
        s1.pull();
      } else {
        s2.pull();
      }
    }
    return true;
  }

  // TODO: Move to dd_func

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
  inline shared_levelized_file<arc>
  transpose(const dd_t &dd)
  {
    adiar_debug(!dd->is_terminal(),
                "Given diagram must be a non-terminal to transpose it.");

    shared_levelized_file<arc> af;

    // Create the contents of 'af'
    { arc_writer aw(af);
      { // Split every node into their arcs.
        node_stream ns(dd);
        while (ns.can_pull()) {
          const typename dd_t::node_t n = ns.pull();

          // TODO (non-binary nodes):
          //   This requires us to extend the 'ptr_t' to have more than a boolean
          //   flag with the out-index.
          aw << low_arc_of(n);
          aw << high_arc_of(n);
        }
      }
      { // Copy over meta information
        af->max_1level_cut = dd->max_1level_cut[cut_type::INTERNAL];

        level_info_stream<> lis(dd);
        while (lis.can_pull()) {
          aw << lis.pull();
        }
      }
    }

    // Sort internal arcs by their target
    af->sort<arc_target_lt>(file_traits<arc>::IDX__INTERNAL);
    return af;
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Converts a `begin` and `end` iterator pair into a generator
  ///        function.
  ////////////////////////////////////////////////////////////////////////////
  template<typename label_t, typename IT>
  inline std::function<label_t()>
  iterator_gen(IT &begin, IT &end)
  {
    return [&begin, &end]() {
      if (begin == end) { return static_cast<label_t>(-1); }
      return static_cast<label_t>(*(begin++));
    };
  }
}

#endif // ADIAR_INTERNAL_UTIL_H

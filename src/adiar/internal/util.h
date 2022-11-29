#ifndef ADIAR_INTERNAL_UTIL_H
#define ADIAR_INTERNAL_UTIL_H

#include <adiar/internal/io/levelized_file.h>
#include <adiar/internal/io/levelized_file_stream.h>
#include <adiar/internal/io/file_writer.h>
#include <adiar/internal/dd.h>

namespace adiar::internal
{
  template<typename T>
  inline ptr_uint64::label_t __label_of(const T& t)
  { return label_of(t); }

  template<>
  inline ptr_uint64::label_t __label_of(const ptr_uint64::label_t& l)
  { return l; }

  template<typename in1_t = dd, typename stream1_t = level_info_stream<>,
           typename in2_t = dd, typename stream2_t = level_info_stream<>>
  bool disjoint_labels(const in1_t &in1, const in2_t &in2)
  {
    stream1_t s1(in1);
    stream2_t s2(in2);

    while(s1.can_pull() && s2.can_pull()) {
      if (__label_of<>(s1.peek()) == __label_of<>(s2.peek())) {
        return false;
      } else if (__label_of<>(s1.peek()) < __label_of<>(s2.peek())) {
        s1.pull();
      } else {
        s2.pull();
      }
    }
    return true;
  }

  ////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain the level_info stream projected onto the labels.
  ////////////////////////////////////////////////////////////////////////////
  // TODO: move to <adiar/internal/dd.h>
  inline label_file varprofile(const dd &dd)
  {
    level_info_stream<> info_stream(dd);

    label_file vars;
    label_writer writer(vars);

    while(info_stream.can_pull()) {
      writer << label_of(info_stream.pull());
    }
    return vars;
  }
}

#endif // ADIAR_INTERNAL_UTIL_H

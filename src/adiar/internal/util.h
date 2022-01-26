#ifndef ADIAR_INTERNAL_UTIL_H
#define ADIAR_INTERNAL_UTIL_H

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/decision_diagram.h>

namespace adiar
{
  template<typename T>
  inline label_t __label_of(const T& t)
  { return label_of(t); }

  template<>
  inline label_t __label_of(const label_t& l)
  { return l; }

  template<typename in1_t = decision_diagram, typename stream1_t = level_info_stream<node_t>,
           typename in2_t = decision_diagram, typename stream2_t = level_info_stream<node_t>>
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

  inline label_file varprofile(const decision_diagram &dd)
  {
    label_file vars;
    label_writer writer(vars);
    level_info_stream<node_t> info_stream(dd);
    while(info_stream.can_pull())
    {
      writer << label_of(info_stream.pull());
    }
    return vars;
  }

}

#endif // ADIAR_INTERNAL_UTIL_H

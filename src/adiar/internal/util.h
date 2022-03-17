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

  class bits_approximation
  {
  public:
    static inline size_t log_2(size_t x)
    {
      unsigned bits, var = x;
      for(bits = 0; var != 0; ++bits) var >>= 1;
      return bits;
    }

  public:
    const size_t bits;

  public:
    ////////////////////////////////////////////////////////////////////////////
    /// An over-approximation of the number of bits to represent a number
    ////////////////////////////////////////////////////////////////////////////
    bits_approximation(const size_t size)
      : bits_approximation(size, false)
    { }

  private:
    bits_approximation(const size_t size, const bool skip_log_2)
      : bits(skip_log_2 ? size : bits_approximation::log_2(size))
    { }

  public:
    bool may_overflow() const
    {
      constexpr size_t max_bits = sizeof(size_t) * 8;
      return max_bits < bits;
    }

  private:
    inline bits_approximation approximate_addition(const size_t bits_1,
                                                   const size_t bits_2) const
    {
      size_t new_bits = std::max(bits_1, bits_2) + 1;
      return bits_approximation(new_bits, true);
    }

    inline bits_approximation approximate_multiplication(const size_t bits_1,
                                                         const size_t bits_2) const
    {
      size_t new_bits = bits_1 + bits_2;
      return bits_approximation(new_bits, true);
    }

  public:
    bits_approximation operator+ (const bits_approximation other) const
    {
      return approximate_addition(bits, other.bits);
    }

    bits_approximation operator* (const bits_approximation other) const
    {
      return approximate_multiplication(bits, other.bits);
    }
  };
}

#endif // ADIAR_INTERNAL_UTIL_H

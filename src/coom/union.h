#ifndef COOM_UNION_H
#define COOM_UNION_H

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// /brief We keep a container for elements of two different types (or none)
  ///
  /// This is similar to an algebraic datatype in functional languages:
  ///
  ///          `type fine_union<T1,T2> = T1 | T2 | None`
  ///
  /// The logic within ensures that only one of the three cases are instantiated
  /// at any point in time.
  ///
  /// TODO: In the implementation, one would notice, that the memory occupied is
  ///
  ///                     sizeof(T1) + sizeof(T2) + X bytes
  ///
  /// Where X has been measured to be anything between 7 to 16 bytes. Maybe
  /// something else from the standard library is better suitable, or we can do
  /// the same explicitly with a C++ union. The perfect scenario would be a
  /// memory footprint of:
  ///
  ///                  max(sizeof(T1), sizeof(T2)) + 1 byte
  ///
  /// Where the last byte is based on a very small number to encode 0, 1, or 2
  /// as the current option to interpret the union with.
  //////////////////////////////////////////////////////////////////////////////
  template<typename T1, typename T2>
  class union_t
  {
    static_assert(!std::is_same<T1, T2>::value);

    // TODO: Use C++ union to minimise space usage?
    std::optional<T1> _t1 = std::nullopt;
    std::optional<T2> _t2 = std::nullopt;

  public:
    union_t() { }

    union_t(const T1 &t1)
    {
      _t1 = t1;
    }

    union_t(const T2 &t2)
    {
      _t2 = t2;
    }

    union_t(const union_t<T1,T2> &fu)
    {
      _t1 = fu._t1;
      _t2 = fu._t2;
    }

    void set(const T1 &t1)
    {
      _t2.reset();
      _t1.emplace(t1);
    }

    void set(const T2 &t2)
    {
      _t1.reset();
      _t2.emplace(t2);
    }

    template<typename T>
    bool has() const
    {
      static_assert(std::is_same<T1, T>::value || std::is_same<T2, T>::value);

      if constexpr (std::is_same<T1, T>::value) {
          return _t1.has_value();
        } else {
        return _t2.has_value();
      }
    }

    template<typename T>
    const T get() const
    {
      static_assert(std::is_same<T1, T>::value || std::is_same<T2, T>::value);

#if COOM_ASSERT
      assert(has<T>());
#endif

      if constexpr (std::is_same<T1, T>::value) {
        return _t1.value();
      } else {
        return _t2.value();
      }
    }
  };

  template<typename T1, typename T2>
  union_t<T1,T2>&
  operator<< (union_t<T1,T2> &fu, const T1 &t1)
  {
    fu.set(t1);
    return fu;
  }

  template<typename T1, typename T2>
  union_t<T1,T2>&
  operator<< (union_t<T1,T2> &fu, const T2 &t2)
  {
    fu.set(t2);
    return fu;
  }
}

#endif

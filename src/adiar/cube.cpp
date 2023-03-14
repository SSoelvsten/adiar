#include "cube.h"

#include <algorithm>

namespace adiar
{
  cube::var_t cube::vars() const
  {
    return this->_file->size();
  }

  bool cube::empty() const
  {
    return this->vars() == 0u;
  }

  cube::var_t cube::dimensionality() const
  {
    cube::var_t s = 0u;
    cube_stream cs(*this);
    while (cs.can_pull()) {
      if (cs.pull().value() == cube::value_t::Dont_Care) { s += 1; }
    }
    return s;
  }

  cube  cube::operator|  (const cube &o) const
  { return cube_union(*this, o); }

  cube& cube::operator|= (const cube &o)
  { return *this = (*this | o); }

  cube  cube::operator&  (const cube &o) const
  { return cube_intsec(*this, o); }

  cube& cube::operator&= (const cube &o)
  { return *this = (*this & o); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Policy-based merge operation of two cubes.
  //////////////////////////////////////////////////////////////////////////////
  template<typename merge_policy>
  cube cube_merge(const cube &A, const cube &B)
  {
    if (A.empty()) {
      return merge_policy::on_empty(B);
    } else if (B.empty()) {
      return merge_policy::on_empty(A);
    }

    shared_file<cube::pair_t> rf;
    file_writer<cube::pair_t> rw(rf);

    cube_stream as(A);
    cube_stream bs(B);

    while (as.can_pull() && bs.can_pull()) {
      cube::pair_t a = as.peek();
      cube::pair_t b = bs.peek();

      if (a.key() != b.key()) {
        if constexpr (merge_policy::include_disjoint) {
          rw << (a.key() < b.key() ? a : b);
        }

        if (a.key() < b.key()) { as.pull(); } else { bs.pull(); }
        continue;
      }

      if (merge_policy::empty_merge(a,b)) {
        as.pull();
        bs.pull();
        continue;
      }

      rw << merge_policy::merge(as.pull(), bs.pull());
    }

    if constexpr (merge_policy::include_disjoint) {
      while (as.can_pull()) { rw << as.pull(); }
      while (bs.can_pull()) { rw << bs.pull(); }
    }
    return rf;
  }

  //////////////////////////////////////////////////////////////////////////////
  class cube_union_policy
  {
  public:
    static inline cube on_empty(const cube &other)
    { return other; }

    static constexpr bool include_disjoint = true;

    static constexpr bool empty_merge(const cube::pair_t&, const cube::pair_t&)
    { return false; }

    static inline cube::pair_t merge(const cube::pair_t& a, const cube::pair_t& b)
    {
      adiar_debug(a.key() == b.key(), "Is only called for matching variables");

      if (a.value() == b.value()) { return a; }
      else { return cube::pair_t(a.key(), cube::value_t::Dont_Care); }
    }
  };

  cube cube_union(const cube &A, const cube &B)
  {
    return cube_merge<cube_union_policy>(A, B);
  }

  //////////////////////////////////////////////////////////////////////////////
  class cube_intsec_policy
  {
  public:
    static inline cube on_empty(const cube &)
    { return cube(); }

    static constexpr bool include_disjoint = false;

    static inline bool empty_merge(const cube::pair_t& a, const cube::pair_t& b)
    {
      adiar_debug(a.key() == b.key(), "Is only called for matching variables");

      return a.value() != b.value()
        && a.value() != cube::value_t::Dont_Care
        && b.value() != cube::value_t::Dont_Care;
    }

    static inline cube::pair_t merge(const cube::pair_t& a, const cube::pair_t& b)
    {
      adiar_debug(a.key() == b.key(), "Is only called for matching variables");

      return cube::pair_t(a.key(), std::min(a.value(), b.value()));
    }
  };

  cube cube_intsec(const cube &A, const cube &B)
  {
    return cube_merge<cube_intsec_policy>(A, B);
  }
}

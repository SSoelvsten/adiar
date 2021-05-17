#include "bdd.h"

#include <memory>

#include <adiar/file_stream.h>
#include <adiar/isomorphism.h>
#include <adiar/reduce.h>

#include <adiar/bdd/apply.h>
#include <adiar/bdd/build.h>
#include <adiar/bdd/negate.h>

namespace adiar {
  __bdd::__bdd(const node_file &f) : union_t(f) { }
  __bdd::__bdd(const arc_file &f) : union_t(f) { }

  __bdd::__bdd(const bdd &bdd) : union_t(bdd.file), negate(bdd.negate) { }


  bdd operator~ (__bdd &&in) { return ~bdd(std::forward<__bdd>(in)); }

  __bdd operator& (__bdd &&lhs, __bdd &&rhs) {
    return bdd(std::forward<__bdd>(lhs)) & bdd(std::forward<__bdd>(rhs));
  }

  __bdd operator| (__bdd &&lhs, __bdd &&rhs) {
    return bdd(std::forward<__bdd>(lhs)) | bdd(std::forward<__bdd>(rhs));
  }

  __bdd operator^ (__bdd &&lhs, __bdd &&rhs) {
    return bdd(std::forward<__bdd>(lhs)) ^ bdd(std::forward<__bdd>(rhs));
  }

  bool operator== (__bdd &&lhs, __bdd &&rhs) {
    return bdd(std::forward<__bdd>(lhs)) == bdd(std::forward<__bdd>(rhs));
  }

  bool operator!= (__bdd &&lhs, __bdd &&rhs) {
    return bdd(std::forward<__bdd>(lhs)) != bdd(std::forward<__bdd>(rhs));
  }

  bool operator== (const bdd &lhs, __bdd &&rhs) {
    return lhs == bdd(std::forward<__bdd>(rhs));
  }

  bool operator!= (const bdd &lhs, __bdd &&rhs) {
    return lhs != bdd(std::forward<__bdd>(rhs));
  }

  bool operator== (__bdd &&lhs, const bdd &rhs) {
    return bdd(std::forward<__bdd>(lhs)) == rhs;
  }

  bool operator!= (__bdd &&lhs, const bdd &rhs) {
    return bdd(std::forward<__bdd>(lhs)) != rhs;
  }

  //////////////////////////////////////////////////////////////////////////////
  node_file reduce(__bdd &&maybe_reduced)
  {
    if (maybe_reduced.has<arc_file>()) {
      node_file out = reduce(maybe_reduced.get<arc_file>());
      maybe_reduced.set(out); // garbage collect arc_file before return
      return out;
    }
    return maybe_reduced.get<node_file>();
  }

  //////////////////////////////////////////////////////////////////////////////
  void bdd::free()
  {
    file._file_ptr.reset();
    negate = false;
  }

  //////////////////////////////////////////////////////////////////////////////
  bdd::bdd(const node_file &f, bool negate) : negate(negate), file(f)
  {
    // TODO: Add adiar_assert on size?
  }

  bdd::bdd(const bdd &o) : negate(o.negate), file(o.file) { }
  bdd::bdd(bool v) : bdd(bdd_sink(v)) { }
  bdd::bdd(bdd &&o) : negate(o.negate), file(o.file) { }

  bdd::bdd(__bdd &&o) : negate(o.negate), file(reduce(std::forward<__bdd>(o))) { }

  //////////////////////////////////////////////////////////////////////////////
  bdd& bdd::operator= (const bdd &other)
  {
    this -> file = other.file;
    this -> negate = other.negate;
    return *this;
  }

  bdd& bdd::operator= (__bdd &&other)
  {
    free();

    // Reduce and move the resulting node_file into our own
    this -> negate = other.negate;
    this -> file = reduce(std::forward<__bdd>(other));
    return *this;
  }

  bdd& bdd::operator&= (const bdd &other)
  {
    return (*this = bdd_and(*this, other));
  }

  bdd& bdd::operator&= (bdd &&other)
  {
    __bdd&& temp = bdd_and(*this, other);
    other.free();
    return (*this = std::move(temp));
  }

  bdd& bdd::operator|= (const bdd &other)
  {
    return (*this = bdd_or(*this, other));
  }

  bdd& bdd::operator|= (bdd &&other)
  {
    __bdd&& temp = bdd_or(*this, other);
    other.free();
    return (*this = std::move(temp));
  }

  bdd& bdd::operator^= (const bdd &other)
  {
    return (*this = bdd_xor(*this, other));
  }

  bdd& bdd::operator^= (bdd &&other)
  {
    __bdd&& temp = bdd_xor(*this, other);
    other.free();
    return (*this = std::move(temp));
  }

  bool operator== (const bdd& lhs, const bdd& rhs)
  {
    return is_isomorphic(lhs.file, rhs.file, lhs.negate, rhs.negate);
  }

  bool operator!= (const bdd& lhs, const bdd& rhs) { return !(lhs == rhs); }

  bdd operator~ (const bdd &in_bdd) { return bdd_not(in_bdd); }
  bdd operator~ (bdd &&in_bdd) { return bdd_not(std::forward<bdd>(in_bdd)); }

  __bdd operator& (const bdd& lhs, const bdd& rhs) { return bdd_and(lhs, rhs); }
  __bdd operator| (const bdd& lhs, const bdd& rhs) { return bdd_or(lhs, rhs); }
  __bdd operator^ (const bdd& lhs, const bdd& rhs) { return bdd_xor(lhs, rhs); }

  //////////////////////////////////////////////////////////////////////////////
  bool is_sink(const bdd &bdd, const sink_pred &pred)
  {
    node_stream<> ns(bdd);
    node_t n = ns.pull();

    return is_sink(n) && pred(n.uid);
  }

  label_t min_label(const bdd &bdd)
  {
    return min_label(bdd.file);
  }

  label_t max_label(const bdd &bdd)
  {
    return max_label(bdd.file);
  }
}

#ifndef COOM_BDD_CPP
#define COOM_BDD_CPP

#include "bdd.h"

#include <memory>

#include <coom/file_stream.h>
#include <coom/homomorphism.h>
#include <coom/reduce.h>

#include <coom/bdd/apply.h>
#include <coom/bdd/negate.h>

namespace coom {
  __bdd::__bdd(const node_file &f) : union_t(f) { }
  __bdd::__bdd(const arc_file &f) : union_t(f) { }

  __bdd::__bdd(const __bdd &o) : union_t(o), negate(o.negate) { }

  __bdd::__bdd(const bdd &bdd) : union_t(bdd.file), negate(bdd.negate) { }

  //////////////////////////////////////////////////////////////////////////////
  node_file reduce(const __bdd &maybe_reduced)
  {
    if (!maybe_reduced.has<node_file>()) {
      return reduce(maybe_reduced.get<arc_file>());
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
  bdd::bdd(const node_file &f, bool negate) : file(f), negate(negate) { }

  bdd::bdd(const bdd &o) : file(o.file), negate(o.negate) { }
  bdd::bdd(bdd &&o) : file(o.file), negate(o.negate) { }

  bdd::bdd(const __bdd &o) : file(reduce(o)), negate(o.negate) { }

  //////////////////////////////////////////////////////////////////////////////
  bdd& bdd::operator= (const bdd &other)
  {
    this -> file = other.file;
    this -> negate = other.negate;
    return *this;
  }

  bdd& bdd::operator= (const __bdd &other)
  {
    free();

    // Reduce and move the resulting node_file into our own
    this -> file = reduce(other);
    this -> negate = other.negate;
    return *this;
  }

  bdd& bdd::operator&= (const bdd &other)
  {
    return (*this = bdd_and(*this, other));
  }

  bdd& bdd::operator&= (bdd &&other)
  {
    __bdd temp = bdd_and(*this, other);
    other.free();
    return (*this = temp);
  }

  bdd& bdd::operator|= (const bdd &other)
  {
    return (*this = bdd_or(*this, other));
  }

  bdd& bdd::operator|= (bdd &&other)
  {
    __bdd temp = bdd_or(*this, other);
    other.free();
    return (*this = temp);
  }

  bdd& bdd::operator^= (const bdd &other)
  {
    return (*this = bdd_xor(*this, other));
  }

  bdd& bdd::operator^= (bdd &&other)
  {
    __bdd temp = bdd_xor(*this, other);
    other.free();
    return (*this = temp);
  }

  bool operator== (const bdd& lhs, const bdd& rhs)
  {
    return is_homomorphic(lhs.file, rhs.file, lhs.negate, rhs.negate);
  }

  bool operator!= (const bdd& lhs, const bdd& rhs) { return !(lhs == rhs); }

  bdd operator~ (const bdd &bdd) { return bdd_not(bdd); }
  bdd operator~ (bdd &&bdd) { return bdd_not(bdd); }

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

#endif // COOM_BDD_CPP

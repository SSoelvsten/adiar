#ifndef ADIAR_BDD_H
#define ADIAR_BDD_H

#include <adiar/union.h>
#include <adiar/file.h>

namespace adiar {
  // Class declarations to be able to reference it
  class bdd;

  //////////////////////////////////////////////////////////////////////////////
  /// An algorithm may return a node-based BDD in a node_file or a yet to-be
  /// reduced BDD in an arc_file. So, the union_t will be a wrapper for the
  /// combined type.
  ///
  /// The union_t class ensures we don't call any expensive yet unnecessary
  /// constructors and ensures only one of the two types are instantiated at a
  /// time.
  //////////////////////////////////////////////////////////////////////////////
  class __bdd : public union_t<node_file, arc_file>
  {
    ////////////////////////////////////////////////////////////////////////////
    // Friends
    friend node_file reduce(__bdd &&maybe_reduced);

    ////////////////////////////////////////////////////////////////////////////
    // Privatize mutating functions from union.h
  private:
    using union_t<node_file, arc_file>::set;

    ////////////////////////////////////////////////////////////////////////////
    // Propagating the negation, when given a bdd.
  public:
    const bool negate = false;

  public:
    ////////////////////////////////////////////////////////////////////////////
    // Constructors
    __bdd(const node_file &f);
    __bdd(const arc_file &f);
    __bdd(const bdd &bdd);
  };

  // operators to allow __bdd&& arguments on one or two sides of an expression
  bdd operator~ (__bdd&&);
  __bdd operator& (__bdd &&, __bdd &&);
  __bdd operator| (__bdd &&, __bdd &&);
  __bdd operator^ (__bdd &&, __bdd &&);
  bool operator== (__bdd &&, __bdd &&);
  bool operator!= (__bdd &&, __bdd &&);
  bool operator== (const bdd &, __bdd &&);
  bool operator!= (const bdd &, __bdd &&);
  bool operator== (__bdd &&, const bdd &);
  bool operator!= (__bdd &&, const bdd &);

  //////////////////////////////////////////////////////////////////////////////
  /// A BDD is the reduced node-based representation together with whether it is
  /// negated or not.
  //////////////////////////////////////////////////////////////////////////////
  class bdd
  {
    ////////////////////////////////////////////////////////////////////////////
    // Friends
    // |- classes
    friend __bdd;

    template <typename T, size_t Files, bool REVERSE>
    friend class meta_stream;

    template <bool REVERSE>
    friend class node_stream;

    template <typename File_T, size_t Files,
              typename T,
              typename LabelExt,
              typename TComparator, typename LabelComparator,
              size_t MetaStreams, size_t Buckets>
    friend class priority_queue;

    // |- functions
    friend bdd bdd_not(const bdd&);
    friend bdd bdd_not(bdd&&);
    friend uint64_t bdd_nodecount(const bdd&);
    friend uint64_t bdd_varcount(const bdd&);

    friend bool is_sink(const bdd &bdd, const sink_pred &pred);
    friend label_t min_label(const bdd &bdd);
    friend label_t max_label(const bdd &bdd);

    // |- operators
    friend bool operator== (const bdd& lhs, const bdd& rhs);
    friend bool operator!= (const bdd& lhs, const bdd& rhs);

    friend bdd operator~ (const bdd& bdd);
    friend bdd operator~ (bdd&& bdd);

    friend __bdd bdd_apply(const bdd &bdd_1, const bdd &bdd_2, const bool_op &op);
    friend __bdd bdd_ite(const bdd &bdd_if, const bdd &bdd_then, const bdd &bdd_else);
    friend __bdd operator& (const bdd &lhs, const bdd &rhs);
    friend __bdd operator| (const bdd &lhs, const bdd &rhs);
    friend __bdd operator^ (const bdd &lhs, const bdd &rhs);

    ////////////////////////////////////////////////////////////////////////////
    // Internal state
  private:
    node_file file;
    bool negate = false;

  private:
    void free();

    ////////////////////////////////////////////////////////////////////////////
    // Constructors
  public:
    bdd(const node_file &f, bool negate = false);

    bdd(bool v);
    bdd(const bdd &o);
    bdd(bdd &&o);

    bdd(__bdd &&o);

    ////////////////////////////////////////////////////////////////////////////
    // Assignment operator overloadings
  public:
    bdd& operator= (const bdd &other);
    bdd& operator= (__bdd &&other);

    bdd& operator&= (const bdd &other);
    bdd& operator&= (bdd &&other);

    bdd& operator|= (const bdd &other);
    bdd& operator|= (bdd &&other);

    bdd& operator^= (const bdd &other);
    bdd& operator^= (bdd &&other);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Check whether a given node_file is sink-only and satisfies the
  /// given sink_pred.
  ///
  /// \param file   The node_file to check its content
  /// \param pred   If the given BDD is sink-only, then secondly the sink is
  ///               checked with the given sink predicate. Default is any type
  ///               sink.
  //////////////////////////////////////////////////////////////////////////////
  bool is_sink(const bdd &bdd, const sink_pred &pred = is_any);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the minimal occurring label in the bdd
  //////////////////////////////////////////////////////////////////////////////
  label_t min_label(const bdd &bdd);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the maximal occurring label in the bdd
  //////////////////////////////////////////////////////////////////////////////
  label_t max_label(const bdd &bdd);
}

#endif // ADIAR_BDD_H

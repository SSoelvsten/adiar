#ifndef COOM_BDD_H
#define COOM_BDD_H

#include <coom/union.h>
#include <coom/file.h>

namespace coom {
  // Class declarations to be able to befriend them
  class bdd;

  //////////////////////////////////////////////////////////////////////////////
  /// An algorithm may return a node-based BDD in a node_file or a yet to-be
  /// reduced BDD in an arc_file. So, the union_t will be a wrapper for the
  /// combined type.
  ///
  /// The union_t class (see union.h) uses the std::optional to ensure we don't
  /// call any expensive yet unnecessary constructors.
  //////////////////////////////////////////////////////////////////////////////
  class __bdd : public union_t<node_file, arc_file>
  {
    ////////////////////////////////////////////////////////////////////////////
    // Propagating the negation, when given a bdd.
  public:
    bool negate = false;

  public:
    ////////////////////////////////////////////////////////////////////////////
    // Constructors
    __bdd();

    __bdd(const node_file &f);
    __bdd(const arc_file &f);

    __bdd(const __bdd &bdd);

    __bdd(const bdd &bdd);

    ////////////////////////////////////////////////////////////////////////////
    // Methods and operators
  public:
    void set(const bdd &bdd);

    __bdd& operator << (const bdd &other);
    __bdd& operator << (const arc_file &af);
    __bdd& operator << (const node_file &nf);
  };


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

    friend bool is_sink(const bdd &bdd, const sink_pred &pred);
    friend label_t min_label(const bdd &bdd);
    friend label_t max_label(const bdd &bdd);

    // |- operators
    friend bool operator== (const bdd& lhs, const bdd& rhs);
    friend bool operator!= (const bdd& lhs, const bdd& rhs);

    friend bdd operator~ (const bdd& bdd);
    friend bdd operator~ (bdd&& bdd);

    friend __bdd operator& (const bdd& lhs, const bdd& rhs);
    friend __bdd operator| (const bdd& lhs, const bdd& rhs);
    friend __bdd operator^ (const bdd& lhs, const bdd& rhs);

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

    bdd(const bdd &o);
    bdd(bdd &&o);

    bdd(const __bdd &o);

    ////////////////////////////////////////////////////////////////////////////
    // Assignment operator overloadings
  public:
    bdd& operator= (const bdd &other);
    bdd& operator= (const __bdd &other);

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
  /// \param pred   If the given OBDD is sink-only, then secondly the sink is
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

#endif // COOM_BDD_H

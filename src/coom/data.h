#ifndef COOM_DATA_H
#define COOM_DATA_H

#include <stdint.h>

namespace coom {
  //////////////////////////////////////////////////////////////////////////////
  /// To condense almost everything down to mere integer comparisons we reserve
  /// specific parts of a single 64 bit unsigned integer to different variables.
  ///
  ///   | S | ???????????????????????????????????????????????????? | F |
  ///
  /// Where these four parts represent the following variables:
  ///
  ///  - S : the is_sink flag. If the sink flag is set, the L and I areas differ
  ///        (see below for the sink type description).
  ///
  ///  - ? : Changes based on whether is_sink flag is set. We guarantee, that
  ///        the ? area will uniquely identify a single element of each type.
  ///
  ///  - F : A boolean flag. This is currently only used in arcs to identify
  ///        high and low arcs (see below).
  ///
  /// An important fact is, that the typedef of ptr and uid below merely are
  /// aliases for the unsigned 64 bit integer. They are merely supposed to
  /// support code readability, but the type checker does actually not care.
  ///
  /// We ensure, that the S and ? areas combined uniquely identify all sinks and
  /// nodes. We also notice, that sorting these pointers directly enforce sink
  /// pointers are sorted after nodes. Finally, two pointers for the same uid
  /// will finally be sorted by the flag.
  //////////////////////////////////////////////////////////////////////////////
  typedef uint64_t ptr_t; // F is possibly set true and/or NIL
  typedef uint64_t uid_t; // F is guaranteed false and never NIL

  bool is_sink_ptr(ptr_t p);
  bool is_node_ptr(ptr_t p);

  bool is_flagged(ptr_t p);
  ptr_t flag(ptr_t p);
  ptr_t unflag(ptr_t p);

  //////////////////////////////////////////////////////////////////////////////
  /// Due to how we create the identifiers for all nodes and sinks, we cannot
  /// use the common null with value 0. Instead we provide a special value that
  /// works with this specific setup.
  //////////////////////////////////////////////////////////////////////////////
  extern const ptr_t NIL;
  bool is_nil(ptr_t p);

  //////////////////////////////////////////////////////////////////////////////
  /// When the is_sink flag is false, then it is a pointer to a node, which is
  /// identifiable by two variables:
  ///
  ///  - L : the variable label. For nodes n1 and n2 with n1.label < n2.label,
  ///        we guarantee that n1 comes before n2 in the stream reading order.
  ///
  ///  - I : a unique identifier for the nodes on the same layer. For nodes n1
  ///        and n2 with n1.label == n2.label but n1.id < n2.id, we guarantee
  ///        that n1 comes before n2 in the stream reading order.
  ///
  /// These are spaced out in the middle area as follows
  ///
  ///   | S | LLLLLLLLLLLLLLLLLLLL | IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII | F |
  ///
  /// That means that nodes are to be sorted first by their label, and then by
  /// their layer-identifier.
  //////////////////////////////////////////////////////////////////////////////
  extern const uint64_t MAX_LABEL;
  extern const uint64_t MAX_ID;

  uid_t create_node_uid(uint64_t label, uint64_t id);

  ptr_t create_node_ptr(uint64_t label, uint64_t id);
  ptr_t create_node_ptr(uid_t uid_t);

  uint64_t label_of(ptr_t p);
  uint64_t label_of(uid_t u);
  uint64_t id_of(ptr_t p);
  uint64_t id_of(uid_t u);

  //////////////////////////////////////////////////////////////////////////////
  /// When the sink flag is set, then we interpret the middle bits as the value
  /// of the sink.
  ///
  ///     | S | VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV | F |
  ///
  /// Notice, that this means we will never have to actually visit to retrieve
  /// its value. That is, the only time a sink has to be explicitly represented
  /// as a node is when the OBDD only consists of said sink.
  //////////////////////////////////////////////////////////////////////////////
  ptr_t create_sink_ptr(bool v);

  bool value_of(ptr_t p);
  bool value_of(uid_t u);

  //////////////////////////////////////////////////////////////////////////////
  /// A node then contains a unique identifier for said node in n.uid_t together
  /// with pointers to its children in n.low and n.high.
  //////////////////////////////////////////////////////////////////////////////
  struct node
  {
    uid_t uid;
    ptr_t low;
    ptr_t high;
  };

  typedef node node_t;

  node create_node(uint64_t label, uint64_t id, ptr_t low, ptr_t high);
  node create_node(uid_t uid_t, ptr_t low, ptr_t high);
  uint64_t id_of(const node& n);
  uint64_t label_of(const node& n);

  node create_sink(bool value);
  bool is_sink(const node& n);
  bool value_of(const node& n);

  bool operator< (const node& a, const node& b);
  bool operator> (const node& a, const node& b);
  bool operator== (const node& a, const node& b);
  bool operator!= (const node& a, const node& b);

  //////////////////////////////////////////////////////////////////////////////
  /// An arc contains a value for the source and one for the target. Notice,
  /// that we don't specifically have a member for is_high. The reason for this
  /// choice is that the C++ compiler makes everything word-aligned. That means
  /// with an explicit is_high member it would take up 3 x 64 bits rather than
  /// only 2 x 64 bits.
  ///
  /// This is finally where the flag above becomes important. The flag is used
  /// on `source` to mark whether the arc is a high or a low arc.
  //////////////////////////////////////////////////////////////////////////////
  struct arc
  {
    ptr_t source;
    ptr_t target;
  };

  typedef arc arc_t;

  bool is_high(arc& a);

  bool operator== (const arc& a, const arc& b);
  bool operator!= (const arc& a, const arc& b);

  //////////////////////////////////////////////////////////////////////////////
  /// Finally, we can create some converters back and forth
  //////////////////////////////////////////////////////////////////////////////
  arc low_arc_of(const node& n);
  arc high_arc_of(const node& n);

  node node_of(const arc& low, const arc& high);
}


#endif // COOM_DATA_H

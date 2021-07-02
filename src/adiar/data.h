#ifndef ADIAR_DATA_H
#define ADIAR_DATA_H

#include <stdint.h>
#include <functional>

#include <tpie/tpie.h>
#include <tpie/file_stream.h>

namespace adiar {
  //////////////////////////////////////////////////////////////////////////////
  /// To condense almost everything down to mere integer comparisons we reserve
  /// specific parts of a single 64 bit unsigned integer to different variables.
  ///
  ///   | S | ???????????????????????????????????????????????????? | F |
  ///
  /// Where these three parts represent the following variables:
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

  bool is_sink(ptr_t p);
  bool is_node(ptr_t p);

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
  ///  - I : a unique identifier for the nodes on the same level. For nodes n1
  ///        and n2 with n1.label == n2.label but n1.id < n2.id, we guarantee
  ///        that n1 comes before n2 in the stream reading order.
  ///
  /// These are spaced out in the middle area as follows
  ///
  ///   | S | LLLLLLLLLLLLLLLLLLLL | IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII | F |
  ///
  /// That means that nodes are to be sorted first by their label, and then by
  /// their level-identifier.
  //////////////////////////////////////////////////////////////////////////////
  extern const uint64_t MAX_LABEL;
  extern const uint64_t MAX_ID;

  typedef uint32_t label_t;
  typedef uint64_t id_t;

  uid_t create_node_uid(label_t label, id_t id);

  ptr_t create_node_ptr(label_t label, id_t id);
  ptr_t create_node_ptr(uid_t uid_t);

  label_t label_of(ptr_t p);
  label_t label_of(uid_t u);
  id_t id_of(ptr_t p);
  id_t id_of(uid_t u);

  //////////////////////////////////////////////////////////////////////////////
  /// When the sink flag is set, then we interpret the middle bits as the value
  /// of the sink.
  ///
  ///     | S | VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV | F |
  ///
  /// Notice, that this means we will never have to actually visit to retrieve
  /// its value. That is, the only time a sink has to be explicitly represented
  /// as a node is when the BDD only consists of said sink.
  //////////////////////////////////////////////////////////////////////////////
  uid_t create_sink_uid(bool v);
  ptr_t create_sink_ptr(bool v);

  bool value_of(ptr_t p);
  bool value_of(uid_t u);

  ptr_t negate(ptr_t p);
  uid_t negate(uid_t u);

  typedef std::function<bool(ptr_t)> sink_pred;

  extern const sink_pred is_any;
  extern const sink_pred is_true;
  extern const sink_pred is_false;

  //////////////////////////////////////////////////////////////////////////////
  /// Using the above functions, we can define all the common binary operators
  /// as functions working on pointers to sinks. These functions will always
  /// return a sink pointer, where the flag bit is set to false.
  ///
  /// Using the knowledge of the bit layout, the following operators are
  /// implemented in three or four fast bit operations:
  ///
  ///                          and, nand, or, nor, xor
  ///
  /// Whereas the rest do their computation using the value_of function defined
  /// above, which are then going to be marginally slower.
  //////////////////////////////////////////////////////////////////////////////
  typedef std::function<ptr_t(ptr_t,ptr_t)> bool_op;

  extern const bool_op and_op;
  extern const bool_op nand_op;
  extern const bool_op or_op;
  extern const bool_op nor_op;
  extern const bool_op xor_op;
  extern const bool_op xnor_op;
  extern const bool_op imp_op;
  extern const bool_op invimp_op;
  extern const bool_op equiv_op;
  extern const bool_op diff_op;
  extern const bool_op less_op;

  //////////////////////////////////////////////////////////////////////////////
  /// At which point we can provide the following predicates for the operators
  ///
  /// - can_right_shortcut:      is op(T, sink) = op(F, sink) ?
  ///
  /// - can_left_shortcut:       is op(sink, T) = op(sink, F) ?
  ///
  /// - is_right_irrelevant:     is op(X, sink) = X ?
  ///
  /// - is_left_irrelevant:      is op(sink, X) = X ?
  ///
  /// - is_right_negating:       is op(X, sink) = ~X ?
  ///
  /// - is_left_negating:        is op(sink, X) = ~X ?
  ///
  /// - is_commutative:          is op(x,y) = op(y,x) ?
  //////////////////////////////////////////////////////////////////////////////
  bool can_right_shortcut(const bool_op &op, const ptr_t sink);
  bool can_left_shortcut(const bool_op &op, const ptr_t sink);

  bool is_right_irrelevant(const bool_op &op, const ptr_t sink);
  bool is_left_irrelevant(const bool_op &op, const ptr_t sink);

  bool is_right_negating(const bool_op &op, const ptr_t sink);
  bool is_left_negating(const bool_op &op, const ptr_t sink);

  bool is_commutative(const bool_op &op);

  //////////////////////////////////////////////////////////////////////////////
  bool on_level(ptr_t p, label_t level);

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

  node_t create_node(uid_t uid_t, ptr_t low, ptr_t high);

  node_t create_node(label_t label, id_t id, ptr_t low, ptr_t high);
  node_t create_node(label_t label, id_t id, ptr_t low, node_t high);
  node_t create_node(label_t label, id_t id, node_t low, ptr_t high);
  node_t create_node(label_t label, id_t id, node_t low, node_t high);

  label_t label_of(const node_t &n);
  id_t id_of(const node_t &n);

  node_t create_sink(bool value);
  bool is_sink(const node_t &n);
  bool value_of(const node_t &n);

  bool on_level(const node_t &n, label_t level);

  node_t negate(const node_t& n);
  node operator! (const node& a);

  bool operator< (const node &a, const node &b);
  bool operator> (const node &a, const node &b);
  bool operator== (const node &a, const node &b);
  bool operator!= (const node &a, const node &b);


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

  // TODO: Create constructor for assert checks on being sorted?

  bool is_high(const arc_t &a);

  arc_t negate(const arc_t& a);
  arc operator! (const arc& a);

  bool operator== (const arc &a, const arc &b);
  bool operator!= (const arc &a, const arc &b);

  struct arc_source_lt : public std::binary_function<arc_t, arc_t, bool>
  {
    bool operator ()(const arc_t& a, const arc_t& b) const {
      return a.source < b.source;
    }
  };

  struct arc_target_lt : public std::binary_function<arc_t, arc_t, bool>
  {
    bool operator ()(const arc_t& a, const arc_t& b) const {
      return a.target < b.target;
    }
  };

  struct arc_target_label
  {
    label_t label_of(const arc_t& a) const {
      return adiar::label_of(a.target);
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// Finally, we can create some converters back and forth
  //////////////////////////////////////////////////////////////////////////////
  arc_t low_arc_of(const node_t& n);
  arc_t high_arc_of(const node_t& n);

  node_t node_of(const arc_t &low, const arc_t &high);


  //////////////////////////////////////////////////////////////////////////////
  /// An assignment to a variable consists of its label and the value of its
  /// assignment.
  ///
  /// TODO: We know that the label is only 16 bits, so we could place the
  /// boolean value inside of the label.
  //////////////////////////////////////////////////////////////////////////////
  struct assignment {
    label_t label;
    bool value;
  };

  typedef assignment assignment_t;

  assignment_t create_assignment(label_t label, bool value);

  assignment operator! (const assignment& a);

  bool operator< (const assignment& a, const assignment& b);
  bool operator> (const assignment& a, const assignment& b);
  bool operator== (const assignment& a, const assignment& b);
  bool operator!= (const assignment& a, const assignment& b);


  //////////////////////////////////////////////////////////////////////////////
  /// Our level-aware priority queue needs to manage which bucket corresponds to
  /// which label to place things correctly or in an internal priority queue for
  /// bucket overflows. To make things more efficient, we require the use of
  /// meta information about the input BDD streams.
  //////////////////////////////////////////////////////////////////////////////
  struct meta
  {
    uint64_t level_info;
  };

  typedef meta meta_t;

  meta_t create_meta(label_t label, size_t level_size);

  label_t label_of(const meta_t &m);
  size_t size_of(const meta_t &m);

  meta operator! (const meta& m);

  bool operator== (const meta &a, const meta &b);
  bool operator!= (const meta &a, const meta &b);
}

#endif // ADIAR_DATA_H

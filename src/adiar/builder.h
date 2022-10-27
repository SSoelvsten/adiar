#ifndef ADIAR_BUILDER_H
#define ADIAR_BUILDER_H

////////////////////////////////////////////////////////////////////////////////
/// \defgroup module__builder Builder
///
/// \brief Manual bottom-up construction of decision diagrams.
///
/// In some cases, one may already know the shape of a \ref bdd and/or as a \ref
/// zdd for a more complex function. In those cases, it is much cheaper to
/// construct them by hand than to manipulate logic formulas.
///
////////////////////////////////////////////////////////////////////////////////

#include <adiar/label.h>
#include <adiar/file_writer.h>

#include <adiar/bdd/bdd_policy.h>
#include <adiar/zdd/zdd_policy.h>

#include <adiar/internal/data_types/uid.h>
#include <adiar/internal/data_types/node.h>

namespace adiar {
  //////////////////////////////////////////////////////////////////////////////
  /// \addtogroup module__builder
  ///
  /// \{

  /// \cond
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Shared information between the builder and its pointers.
  ///
  /// \see builder buider_ptr
  //////////////////////////////////////////////////////////////////////////////
  struct builder_shared
  {

  };
  /// \endcond

  template<typename dd_policy>
  class builder;

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief The pointer type that builders use to identify the nodes they have
  ///        constructed in a decision diagram.
  ///
  /// \see builder
  ///////////////////////////////////////////////////////////////////////////////
  template<typename dd_policy>
  class builder_ptr
  {
  public:
    friend class builder<dd_policy>;

  private:
    ///////////////////////////////////////////////////////////////////////////////
    /// \brief The unique identifier of a prior node.
    ///////////////////////////////////////////////////////////////////////////////
    // TODO: rename to 'ptr ptr' when using complement edges
    /*const*/ uid_t uid;

    ///////////////////////////////////////////////////////////////////////////////
    /// \brief Unique shared reference for the parent builder object.
    ///////////////////////////////////////////////////////////////////////////////
    /*const*/ std::shared_ptr<const builder_shared> builder_ref;

    ///////////////////////////////////////////////////////////////////////////////
    /// \brief Reference for this specific 'uid' recording whether it is a child
    ///        of a node created later than itself.
    ///////////////////////////////////////////////////////////////////////////////
    /*const*/ std::shared_ptr<bool> unreferenced = std::make_shared<bool>(true);

  public:
    builder_ptr() = default;
    builder_ptr(const builder_ptr&) = default;

  private:
    builder_ptr(const uid_t p, const std::shared_ptr<const builder_shared> &sp)
      : uid(p), builder_ref(sp)
    { }
  };

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Pointer for a BDD node created by a BDD builder.
  ///
  /// \see bdd_builder
  ///////////////////////////////////////////////////////////////////////////////
  typedef builder_ptr<bdd_policy> bdd_ptr;

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Pointer for a ZDD node created by a ZDD builder.
  ///
  /// \see zdd_builder
  ///////////////////////////////////////////////////////////////////////////////
  typedef builder_ptr<zdd_policy> zdd_ptr;


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief A builder for decision diagrams
  ///
  /// \details Nodes should be added bottom-up. Each added node returns a
  ///          builder_ptr so that it can be used as a child for other nodes.
  ///
  /// \tparam dd_policy Logic related to the specific type of decision diagram
  ///                   to construct.
  ///////////////////////////////////////////////////////////////////////////////
  template<typename dd_policy>
  class builder
  {
  private:
    // TODO: code duplication with .reset().

    /////////////////////////////////////////////////////////////////////////////
    /// \brief File containing all prior pushed nodes.
    /////////////////////////////////////////////////////////////////////////////
    node_file nf;

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Node writer to push new nodes into 'nf'.
    /////////////////////////////////////////////////////////////////////////////
    node_writer nw;

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Whether a terminal value has been returned in 'add_node'.
    /////////////////////////////////////////////////////////////////////////////
    bool created_terminal = false;

    /////////////////////////////////////////////////////////////////////////////
    /// \brief The value of the latest terminal value returned in 'add_node'.
    /////////////////////////////////////////////////////////////////////////////
    bool terminal_val = false;

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Label of the current level.
    /////////////////////////////////////////////////////////////////////////////
    label_t current_label = MAX_LABEL;

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Next available level identifier for the current level.
    /////////////////////////////////////////////////////////////////////////////
    id_t current_id = MAX_ID;

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Unique struct for this builder's current phase.
    /////////////////////////////////////////////////////////////////////////////
    std::shared_ptr<builder_shared> builder_ref = std::make_shared<builder_shared>();

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Number of yet unreferenced nodes.
    /////////////////////////////////////////////////////////////////////////////
    size_t unref_nodes = 0u;

  public:
    builder() : nw(nf)
    { }

  public:
    /////////////////////////////////////////////////////////////////////////////
    /// \brief Add an internal node with a given label, id, and its two children.
    ///
    /// \param label The variable label for the node to create.
    ///
    /// \param low   Pointer for when the variable with the given label evaluates
    ///              to `false`.
    ///
    /// \param high  Pointer for when the variable with the given label evaluates
    ///              to `true`.
    ///
    /// \returns     Pointer to the (possibly new) node for the desired
    ///              function.
    ///
    /// \remark This will apply the first reduction rule associated with the
    ///         specific type of decision diagram, e.g. not construct a "don’t
    ///         care" node for \ref bdd but instead just return its child. The
    ///         builder cannot apply the second reduction rule, i.e. merging of
    ///         duplicate nodes, so that is still left up to you to do.
    ///
    /// \throws std::invalid_argument If a node is malformed or not added in
    ///                               bottom-up order and if pointers from
    ///                               another builder is used.
    /////////////////////////////////////////////////////////////////////////////
    builder_ptr<dd_policy> add_node(label_t label,
                                    const builder_ptr<dd_policy> &low,
                                    const builder_ptr<dd_policy> &high)
    {
      // Check validity of input
      if(low.builder_ref != builder_ref || high.builder_ref != builder_ref) {
        throw std::invalid_argument("Cannot use pointers from a different builder");
      }
      if(label > MAX_LABEL) {
        throw std::invalid_argument("Nodes must have a valid label");
      }
      if(label > current_label) {
        throw std::invalid_argument("Nodes must be added bottom-up");
      }
      if(low.uid.is_node() && low.uid.label() <= label) {
        throw std::invalid_argument("Low child must point to a node with higher label");
      }
      if(high.uid.is_node() && high.uid.label() <= label) {
        throw std::invalid_argument("High child must point to a node with higher label");
      }

      // Update label and ID if necessary
      if(label < current_label) {
        current_label = label;
        current_id = MAX_ID;
      }

      // Create potential node
      const node_t n(label, current_id, low.uid, high.uid);

      // Check whether this node is 'redundant'
      const uid_t res_uid = dd_policy::reduction_rule(n);

      if (res_uid.is_terminal()) {
        created_terminal = true;
        terminal_val = res_uid.value();
      }

      if(res_uid == low.uid) { return low; }
      if(res_uid == high.uid) { return high; }

      // Push node to file
      nw.push(n);
      unref_nodes++;
      current_id--;

      // Update count of unreferenced nodes
      bool& low_unref = *low.unreferenced;
      if(low_unref && !low.uid.is_terminal()) {
        low_unref = false;
        unref_nodes--;
      }

      bool& high_unref = *high.unreferenced;
      if(high_unref && !high.uid.is_terminal()) {
        high_unref = false;
        unref_nodes--;
      }

      return make_ptr(res_uid);
    }

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Add an internal node with a given label, id, and its two children.
    ///
    /// \param label The variable label for the node to create.
    ///
    /// \param low   Terminal value for when the variable with the given label
    ///              evaluates to `false`.
    ///
    /// \param high  Pointer for when the variable with the given label evaluates
    ///              to `true`.
    ///
    /// \returns     Pointer to the (possibly new) node for the desired
    ///              function.
    ///
    /// \throws std::invalid_argument If `label` and `high` are illegal (see
    ///                               \ref add_node).
    /////////////////////////////////////////////////////////////////////////////
    builder_ptr<dd_policy> add_node(const label_t label,
                                    const bool low,
                                    const builder_ptr<dd_policy> &high)
    {
      builder_ptr<dd_policy> low_ptr = make_ptr(ptr_uint64(low));
      return add_node(label, low_ptr, high);
    }

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Add an internal node with a given label, id, and its two children.
    ///
    /// \param label The variable label for the node to create.
    ///
    /// \param low   Pointer for when when the variable with the given label
    ///              evaluates to `false`.
    ///
    /// \param high  Terminal value for when the variable with the given label
    ///              evaluates to `true`.
    ///
    /// \returns     Pointer to the (possibly new) node for the desired
    ///              function.
    ///
    /// \throws std::invalid_argument If `label` and `low` are illegal (see
    ///                               \ref add_node).
    /////////////////////////////////////////////////////////////////////////////
    builder_ptr<dd_policy> add_node(const label_t label,
                                    const builder_ptr<dd_policy> &low,
                                    const bool high)
    {
      builder_ptr<dd_policy> high_ptr = make_ptr(ptr_uint64(high));
      return add_node(label, low, high_ptr);
    }

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Add an internal node with a given label, id, and its two children.
    ///
    /// \param label The variable label for the node to create.
    ///
    /// \param low   Terminal value for when when the variable with the given
    ///              label evaluates to `false`.
    ///
    /// \param high  Terminal value for when the variable with the given label
    ///              evaluates to `true`.
    ///
    /// \returns     Pointer to the node for the desired function.
    ///
    /// \throws std::invalid_argument If `label` violates the bottom-up ordering
    ///                               (see \ref add_node).
    /////////////////////////////////////////////////////////////////////////////
    builder_ptr<dd_policy> add_node(const label_t label,
                                    const bool low,
                                    const bool high)
    {
      builder_ptr<dd_policy> low_ptr = make_ptr(ptr_uint64(low));
      builder_ptr<dd_policy> high_ptr = make_ptr(ptr_uint64(high));
      return add_node(label, low_ptr, high_ptr);
    }

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Add a terminal node with a given value.
    ///
    /// \param terminal_value Value of the constant function to create.
    ///
    /// \remark One may call this function at any time during the construction
    ///         process since terminals are not part of the *bottom-up*
    ///         requirement.
    /////////////////////////////////////////////////////////////////////////////
    builder_ptr<dd_policy> add_node(bool terminal_value)
    {
      created_terminal = true;
      terminal_val = terminal_value;

      return make_ptr(ptr_uint64(terminal_value));
    }

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Builds the decision diagram with the added nodes. This also
    ///        clears the builder.
    ///
    /// \see clear
    ///
    /// \throws std::domain_error If (a) add_node has not been called or (b) if
    ///                           there are more than one root in the diagram.
    ///
    /// \pre     One has used `add_node` to create a non-empty and singly-rooted
    ///          decision diagram.
    ///
    /// \warning After calling this function all previously created
    ///          `builder_ptr` created by this object will be invalidated and
    ///          cannot be used anymore.
    /////////////////////////////////////////////////////////////////////////////
    typename dd_policy::reduced_t build()
    {
      if(!nw.has_pushed()) {
        if(created_terminal) {
          nw.push(node(terminal_val));
        } else {
          throw std::domain_error("There must be at least one node or terminal in the decision diagram");
        }
      }
      nw.detach();

      if(unref_nodes > 1) {
        throw std::domain_error("Decision diagram has more than one root");
      }

      const typename dd_policy::reduced_t res(nf);
      reset();
      return res;
    }

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Clear builder of all its current content, discarding all nodes
    ///        and invalidating any pointers to them.
    ///
    /// \warning This will invalidate any pointers created up to this point. You
    ///          will not be able to use them after this.
    /////////////////////////////////////////////////////////////////////////////
    void clear() noexcept
    {
      reset();
    }

  private:
    /////////////////////////////////////////////////////////////////////////////
    /// \brief Reset all internal values to their initial.
    /////////////////////////////////////////////////////////////////////////////
    void reset() noexcept
    {
      nw.detach();
      nf = node_file();
      nw.attach(nf);

      current_label = MAX_LABEL;
      current_id = MAX_ID;
      created_terminal = false;
      terminal_val = false;
      builder_ref = std::make_shared<builder_shared>();
      unref_nodes = 0;
    }

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Create a builder_ptr with 'this' builder as its parent.
    /////////////////////////////////////////////////////////////////////////////
    builder_ptr<dd_policy> make_ptr(const ptr_uint64 p) noexcept
    {
      return builder_ptr<dd_policy>(p, builder_ref);
    }
  };

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Builder for BDDs
  ///
  /// \sa builder
  ///////////////////////////////////////////////////////////////////////////////
  typedef builder<bdd_policy> bdd_builder;

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Builder for ZDDs
  ///
  /// \sa builder
  ///////////////////////////////////////////////////////////////////////////////
  typedef builder<zdd_policy> zdd_builder;

  /// \}
  /////////////////////////////////////////////////////////////////////////////
}

#endif // ADIAR_BUILDER_H

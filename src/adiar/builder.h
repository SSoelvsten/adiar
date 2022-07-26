#ifndef ADIAR_BUILDER_H
#define ADIAR_BUILDER_H

#include <adiar/data.h>
#include <adiar/file_writer.h>

#include <adiar/bdd/bdd_policy.h>
#include <adiar/zdd/zdd_policy.h>

namespace adiar {

  struct builder_shared
  {

  };

  template<typename dd_policy>
  class builder;

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief The pointer type that builders use to nodes in decision diagrams
  ///
  /// \sa builder_ptr builder
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
    // TODO: rename to 'ptr_t ptr' when using complement edges
    /*const*/ uid_t uid;

    ///////////////////////////////////////////////////////////////////////////////
    /// \brief Unique shared reference for the parent builder object.
    ///////////////////////////////////////////////////////////////////////////////
    /*const*/ std::shared_ptr<builder_shared> builder_ref;

    ///////////////////////////////////////////////////////////////////////////////
    /// \brief Reference for this specific 'uid' recording whether it is a child
    ///        of a node created later than itself.
    ///////////////////////////////////////////////////////////////////////////////
    /*const*/ std::shared_ptr<bool> unreferenced = std::make_shared<bool>(true);

  public:
    builder_ptr() = default;
    builder_ptr(const builder_ptr&) = default;

  private:
    builder_ptr(uid_t &p, std::shared_ptr<builder_shared> &sp) : uid(p), builder_ref(sp)
    { }
  };

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Pointer for a BDD node created by a BDD builder.
  ///
  /// \sa builder_ptr builder
  ///////////////////////////////////////////////////////////////////////////////
  typedef builder_ptr<bdd_policy> bdd_ptr;

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Pointer for a ZDD node created by a ZDD builder.
  ///
  /// \sa builder_ptr builder
  ///////////////////////////////////////////////////////////////////////////////
  typedef builder_ptr<zdd_policy> zdd_ptr;


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief A builder for decision diagrams
  ///
  /// \details Nodes should be added bottom-up. Each added node returns a
  ///          builder_ptr so that it can be used as a child for other nodes.
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
    /// \brief Add a terminal node with a given value.
    /////////////////////////////////////////////////////////////////////////////
    builder_ptr<dd_policy> add_node(bool terminal_value)
    {
      created_terminal = true;
      terminal_val = terminal_value;

      return make_ptr(create_terminal_ptr(terminal_value));
    }

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Add an internal node with a given label, id, and children.
    ///        Children are either pointers to other nodes or booleans.
    ///
    /// \param label The variable label for the node to create.
    ///
    /// \param low   Pointer or Terminal value for when the variable for the
    ///              given label evaluates to false.
    ///
    /// \param high  Pointer or Terminal value for when the variable for the
    ///              given label evaluates to true.
    ///
    /// \returns     Pointer to the (possibly new) BDD node for the desired
    ///              function.
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
      if(is_node(low.uid) && label_of(low.uid) <= label) {
        throw std::invalid_argument("Low child must point to a node with higher label");
      }
      if(is_node(high.uid) && label_of(high.uid) <= label) {
        throw std::invalid_argument("High child must point to a node with higher label");
      }

      // Update label and ID if necessary
      if(label < current_label) {
        current_label = label;
        current_id = MAX_ID;
      }

      // Create potential node
      const node_t node = create_node(label, current_id, low.uid, high.uid);

      // Check whether this node is 'redundant'
      const uid_t res_uid = dd_policy::reduction_rule(node);

      if (is_terminal(res_uid)) {
        created_terminal = true;
        terminal_val = value_of(res_uid);
      }

      if(res_uid == low.uid) { return low; }
      if(res_uid == high.uid) { return high; }

      // Push node to file
      nw.push(node);
      unref_nodes++;
      current_id--;

      // Update count of unreferenced nodes
      bool& low_unref = *low.unreferenced;
      if(low_unref && !is_terminal(low.uid)) {
        low_unref = false;
        unref_nodes--;
      }

      bool& high_unref = *high.unreferenced;
      if(high_unref && !is_terminal(high.uid)) {
        high_unref = false;
        unref_nodes--;
      }

      return make_ptr(res_uid);
    }

    builder_ptr<dd_policy> add_node(const label_t label,
                                    const bool low,
                                    const builder_ptr<dd_policy> &high)
    {
      builder_ptr<dd_policy> low_ptr = make_ptr(create_terminal_ptr(low));
      return add_node(label, low_ptr, high);
    }

    builder_ptr<dd_policy> add_node(const label_t label,
                                    const builder_ptr<dd_policy> &low,
                                    const bool high)
    {
      builder_ptr<dd_policy> high_ptr = make_ptr(create_terminal_ptr(high));
      return add_node(label, low, high_ptr);
    }

    builder_ptr<dd_policy> add_node(const label_t label,
                                    const bool low,
                                    const bool high)
    {
      builder_ptr<dd_policy> low_ptr = make_ptr(create_terminal_ptr(low));
      builder_ptr<dd_policy> high_ptr = make_ptr(create_terminal_ptr(high));
      return add_node(label, low_ptr, high_ptr);
    }

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Builds the decision diagram with the added nodes and is then
    ///        fully cleared.
    ///
    /// \sa clear
    ///
    /// \throws std::domain_error If (a) add_node has not been called or (b) if
    ///                           there are more than one root in the diagram.
    /////////////////////////////////////////////////////////////////////////////
    typename dd_policy::reduced_t build()
    {
      if(!nw.has_pushed()) {
        if(created_terminal) {
          nw.push(create_terminal(terminal_val));
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
    builder_ptr<dd_policy> make_ptr(const ptr_t p) noexcept
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

}

#endif // ADIAR_BUILDER_H

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
    const ptr_t ptr;
    const std::shared_ptr<builder_shared> builder_ref;
    std::shared_ptr<bool> unreferenced = std::make_shared<bool>(true);

  public:
    builder_ptr() = default;
    builder_ptr(const builder_ptr&) = default;

  private:
    builder_ptr(ptr_t &p, std::shared_ptr<builder_shared> &sp) : ptr(p), builder_ref(sp)
    { }

  };

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief The pointer type for BDD nodes that builders use
  ///
  /// \sa builder_ptr builder
  ///////////////////////////////////////////////////////////////////////////////
  typedef builder_ptr<bdd_policy> bdd_ptr;

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief The pointer type for ZDD nodes that builders use
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
    node_file nf;
    node_writer nw;

    bool created_terminal = false;
    bool terminal_val = false;

    label_t current_label = MAX_LABEL;
    id_t current_id = MAX_ID;

    std::shared_ptr<builder_shared> builder_ref = std::make_shared<builder_shared>();
    size_t unref_nodes = 0;

  public:
    builder() : nw(nf)
    { }

  public:
    /////////////////////////////////////////////////////////////////////////////
    /// \brief Add a terminal node with a given value.
    /////////////////////////////////////////////////////////////////////////////
    builder_ptr<dd_policy> add_node(bool terminal_value) {
      created_terminal = true;
      terminal_val = terminal_value;

      return make_ptr(create_terminal_ptr(terminal_value));
    }

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Add an internal node with a given label, id, and children.
    ///        Children are either pointers to other nodes or booleans.
    /////////////////////////////////////////////////////////////////////////////
    builder_ptr<dd_policy> add_node(label_t label, builder_ptr<dd_policy> &low, builder_ptr<dd_policy> &high)
    {
      //Check validity of input
      if(low.builder_ref != builder_ref || high.builder_ref != builder_ref) {
        throw std::invalid_argument("Cannot use pointers from a different builder");
      }
      if(label > MAX_LABEL) {
        throw std::invalid_argument("Nodes must have a valid label");
      }
      if(label > current_label) {
        throw std::invalid_argument("Nodes must be added bottom-up");
      }
      if(is_node(low.ptr) && label_of(low.ptr) <= label) {
        throw std::invalid_argument("Low child must point to a node with higher label");
      }
      if(is_node(high.ptr) && label_of(high.ptr) <= label) {
        throw std::invalid_argument("High child must point to a node with higher label");
      }

      //Update label and ID if necessary
      if(label < current_label) {
        current_label = label;
        current_id = MAX_ID;
      }

      //Create and push new node
      node_t node = create_node(label, current_id--, low.ptr, high.ptr);
      ptr_t ptr = dd_policy::reduction_rule(node);
      if(ptr == low.ptr) { return low; }
      if(ptr == high.ptr) { return high; }

      nw.push(node);
      unref_nodes++;

      //Update count of unreferenced nodes
      bool& low_unref = *low.unreferenced;
      if(low_unref && !is_terminal(low.ptr)) {
        low_unref = false;
        unref_nodes--;
      }
      bool& high_unref = *high.unreferenced;
      if(high_unref && !is_terminal(high.ptr)) {
        high_unref = false;
        unref_nodes--;
      }

      return make_ptr(ptr);
    }

    builder_ptr<dd_policy> add_node(label_t label, bool low, builder_ptr<dd_policy> &high)
    {
      builder_ptr<dd_policy> low_ptr = make_ptr(create_terminal_ptr(low));
      return add_node(label, low_ptr, high);
    }

    builder_ptr<dd_policy> add_node(label_t label, builder_ptr<dd_policy> &low, bool high)
    {
      builder_ptr<dd_policy> high_ptr = make_ptr(create_terminal_ptr(high));
      return add_node(label, low, high_ptr);
    }

    builder_ptr<dd_policy> add_node(label_t label, bool low, bool high)
    {
      builder_ptr<dd_policy> low_ptr = make_ptr(create_terminal_ptr(low));
      builder_ptr<dd_policy> high_ptr = make_ptr(create_terminal_ptr(high));
      return add_node(label, low_ptr, high_ptr);
    }

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the final decision diagram.
    /////////////////////////////////////////////////////////////////////////////
    typename dd_policy::reduced_t create()
    {
      if(!nw.has_pushed()) {
        if(created_terminal) {
          nw.push(create_terminal(terminal_val));
        } else {
          throw std::domain_error("There must be at least one node or terminal in the decision diagram");
        }
      }
      if(unref_nodes > 1) {
        throw std::domain_error("Decision diagram has more than one root");
      }
      nw.detach();
      return nf;
    }

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Reset to create a new decision diagram.
    /////////////////////////////////////////////////////////////////////////////
    void reset()
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

  private:
    builder_ptr<dd_policy> make_ptr(ptr_t p) {
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

#ifndef ADIAR_INTERNAL_IO_NODE_ARC_RACCESS_H
#define ADIAR_INTERNAL_IO_NODE_ARC_RACCESS_H

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/io/levelized_raccess.h>
#include <adiar/internal/io/node_arc_stream.h>

namespace adiar::internal
{
  class node_arc_raccess : public levelized_raccess<node_arc_stream<>>
  {
    using parent_type = levelized_raccess<node_arc_stream<>>;

  public:
    static size_t
    memory_usage(tpie::memory_size_type max_width)
    {
      // To support overloading with both 'width' and 'diagram', we need both in this class.
      return parent_type::memory_usage(max_width);
    }

    static size_t
    memory_usage(const __dd& diagram)
    {
      adiar_assert(diagram.template has<__dd::shared_arc_file_type>());
      return parent_type::memory_usage(diagram.template get<__dd::shared_arc_file_type>()->width);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a levelized file of arcs.
    ///
    /// \pre The given levelized file is *indexable*.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node_arc_raccess(levelized_file<arc>& f,
                     const bool negate                                   = false,
                     [[maybe_unused]] const arc::signed_label_type shift = 0)
      : parent_type(f, negate)
    {
      // adiar_assert(f.indexable);
      adiar_assert(negate == false);
      adiar_assert(shift == false);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a shared levelized file of arcs.
    ///
    /// \pre The given shared levelized file is *indexable*.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node_arc_raccess(const shared_ptr<levelized_file<arc>>& f,
                     const bool negate                                   = false,
                     [[maybe_unused]] const arc::signed_label_type shift = 0)
      : parent_type(f, negate)
    {
      // adiar_assert(f->indexable);
      adiar_assert(negate == false);
      adiar_assert(shift == false);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to an unreduced decision diagram.
    ///
    /// \pre The unreduced decision diagram is indexable. This is (almost) always the case.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node_arc_raccess(const __dd& diagram)
      : node_arc_raccess(diagram.template get<__dd::shared_arc_file_type>(), diagram._negate)
    {}

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the node at the given index.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const value_type&
    at(idx_type idx) const
    {
      // To properly support overloading with both 'idx' and 'uid', we need both in this class.
      return parent_type::at(idx);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain the node with the given uid.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    const value_type&
    at(uid_type u) const
    {
      adiar_assert(static_cast<signed_label_type>(u.label()) == current_level());
      return parent_type::at(static_cast<idx_type>(u.id()));
    }
  };
}

#endif // ADIAR_INTERNAL_IO_NODE_ARC_RACCESS_H

#ifndef ADIAR_INTERNAL_IO_NODE_RANDOM_ACCESS_H
#define ADIAR_INTERNAL_IO_NODE_RANDOM_ACCESS_H

#include <adiar/internal/assert.h>
#include <adiar/internal/data_types/node.h>
#include <adiar/internal/dd.h>
#include <adiar/internal/io/levelized_random_access.h>
#include <adiar/internal/io/node_stream.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////////////////////////
  /// \brief Random-access to the contents of a levelized file of node.
  //////////////////////////////////////////////////////////////////////////////////////////////////
  class node_random_access : public levelized_random_access<node_stream<>>
  {
    using parent_type = levelized_random_access<node_stream<>>;

  public:
    static size_t
    memory_usage(tpie::memory_size_type max_width)
    {
      // To support overloading with both 'width' and 'diagram', we need both in this class.
      return parent_type::memory_usage(max_width);
    }

    static size_t
    memory_usage(const dd& diagram)
    {
      return parent_type::memory_usage(diagram->width);
    }

  public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a levelized file of nodes.
    ///
    /// \pre The given levelized file is *indexable*.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node_random_access(const levelized_file<value_type>& f,
                       const bool negate                         = false,
                       const node::signed_label_type level_shift = 0)
      : parent_type(f, negate, level_shift)
    {
      adiar_assert(f.indexable);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a shared levelized file of nodes.
    ///
    /// \pre The given shared levelized file is *indexable*.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node_random_access(const shared_ptr<levelized_file<value_type>>& f,
                       const bool negate                         = false,
                       const node::signed_label_type level_shift = 0)
      : parent_type(f, negate, level_shift)
    {
      adiar_assert(f->indexable);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Construct attached to a decision diagram.
    ///
    /// \pre The given decision diagram is indexable.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    node_random_access(const dd& diagram)
      : node_random_access(diagram.file_ptr(), diagram.is_negated())
    {
      adiar_assert(diagram->indexable);
    }

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
      adiar_assert(static_cast<signed_label_type>(u.label()) == this->current_level());

      const idx_type idx = this->current_width() - ((uid_type::max_id + 1u) - u.id());
      return parent_type::at(idx);
    }
  };
}

#endif // ADIAR_INTERNAL_IO_NODE_RANDOM_ACCESS_H

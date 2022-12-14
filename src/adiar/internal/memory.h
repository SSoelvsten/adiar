#ifndef ADIAR_INTERNAL_MEMORY_H
#define ADIAR_INTERNAL_MEMORY_H

#include <string>

#include <tpie/tpie.h>
#include <tpie/memory.h>

#include <adiar/internal/assert.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain from TPIE the amount of available memory.
  //////////////////////////////////////////////////////////////////////////////
  inline size_t memory_available()
  {
    return tpie::get_memory_manager().available();
  }
}

namespace adiar
{
  // Based on <tpie/memory.h>

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Shared ownership of an object on the heap. This *smart pointer*
  ///        essentially provides a (thread-safe) reference count and automatic
  ///        garbage collection.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  using shared_ptr = std::shared_ptr<T>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Creates a new object on the heap with shared ownership.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, typename ... TT>
  inline shared_ptr<T> make_shared(TT && ... tt) {
    return std::make_shared<T>(std::forward<TT>(tt)...);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sole ownership of an object on the heap. Unlike the `shared_ptr`,
  ///        this *smart pointer* has the semantics of a single owner (and hence
  ///        no need to do reference counting).
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  using unique_ptr = std::unique_ptr<T>;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Creates a new object on the heap with unique ownership.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, typename ... TT>
  inline unique_ptr<T> make_unique(TT && ... tt) {
    return std::make_unique<T>(std::forward<TT>(tt)...);
  }
}

#endif // ADIAR_INTERNAL_MEMORY_H

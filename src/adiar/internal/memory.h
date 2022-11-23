#ifndef ADIAR_INTERNAL_MEMORY_H
#define ADIAR_INTERNAL_MEMORY_H

#include <string>

#include <tpie/tpie.h>
#include <tpie/memory.h>

#include <adiar/internal/assert.h>

namespace adiar::internal
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Minimum value of 128 MiB for the memory limit.
  ///
  /// \ingroup module__adiar
  //////////////////////////////////////////////////////////////////////////////
  constexpr size_t MINIMUM_MEMORY = 128 * 1024 * 1024;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sets the memory limit for TPIE.
  ///
  /// \throws std::invalid_argument if the amount of memory given is smaller
  ///         than \ref MINIMUM_BYTES
  //////////////////////////////////////////////////////////////////////////////
  inline void memory_set_limit(size_t memory_limit_bytes = MINIMUM_MEMORY)
  {
    if (memory_limit_bytes < MINIMUM_MEMORY) {
      throw std::invalid_argument("Adiar requires at least "
                                  + std::to_string(MINIMUM_MEMORY / 1024 / 1024)
                                  + " MiB of memory");
    }

    tpie::get_memory_manager().set_limit(memory_limit_bytes);
  }

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

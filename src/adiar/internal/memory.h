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

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The largest value that TPIE can use without some computation for
  /// internal memory breaking.
  ///
  /// TPIE's data structures provide the `memory_usage` and `memory_fits`
  /// functions. These cast the `tpie::memory_size_type` (an alias for
  /// `std::size_t`) into a `double`, do some computations and then cast them
  /// back into a `tpie::memory_size_type`.
  ///
  /// Yet, on Clang with specific optimisations enabled some parts of this
  /// computation is cast to an `unsigned long` (32 bits) which leads to
  /// undefined behaviour. Due to this, `memory_usage(memory_fits(x))` is
  /// unexpectedly larger than `x`.
  ///
  /// This value is an empirically derived value for a `tpie::array<int>` and is
  /// equivalent to 4 PiB for which these computations by TPIE are safe.
  ///
  /// \see priority_queue sorter
  //////////////////////////////////////////////////////////////////////////////
  constexpr tpie::memory_size_type tpie_max_bytes =
    std::numeric_limits<tpie::memory_size_type>::max() >> 12;
}

namespace adiar
{
  // TODO: add std::move(...) alias

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

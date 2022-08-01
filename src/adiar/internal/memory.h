#ifndef ADIAR_INTERNAL_MEMORY_H
#define ADIAR_INTERNAL_MEMORY_H

#include <string>

#include <tpie/tpie.h>
#include <tpie/memory.h>
#include <tpie/tempname.h>

#include <adiar/internal/assert.h>

namespace adiar
{
  namespace memory
  {
    //////////////////////////////////////////////////////////////////////////////
    /// \addtogroup module__adiar
    ///
    /// \{

    //////////////////////////////////////////////////////////////////////////////
    /// \name Memory Mode
    ///
    /// Adiar’s Algorithms delay recursion through use of auxiliary data
    /// structures. These auxiliary data structures can be optimised for
    /// internal memory, and so have a high performance on very small instances,
    /// or they can be designed for external memory such that they can handle
    /// decision diagrams much larger than the available memory.
    ///
    /// \{

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Whether Adiar should exclusively use internal or external memory or
    ///        automatically pick either type based on size of input.
    ///
    /// \details
    ///   | Enum Value | Effect                                         |
    ///   |------------|------------------------------------------------|
    ///   | `AUTO`     | Automatically decide on type of memory to use. |
    ///   | `INTERNAL` | Always use *internal* memory.                  |
    ///   | `EXTERNAL` | Always use *external* memory.                  |
    //////////////////////////////////////////////////////////////////////////////
    enum memory_mode { AUTO, INTERNAL, EXTERNAL };

    //////////////////////////////////////////////////////////////////////////////
    /// \brief The current memory mode (default: `AUTO`).
    ///
    /// \details
    ///   If you want to force *Adiar* to a specific memory mode then you should set the
    ///   global variable `adiar::memory::mode` to one of the above three values. For
    ///   example, one can force *Adiar* always use internal memory with the
    ///   following piece of code.
    ///   ```cpp
    ///   adiar::memory::mode = adiar::memory::INTERNAL
    ///   ```
    ///
    /// \warning Using `INTERNAL` may lead to crashes if an input or output is
    ///          too large!
    //////////////////////////////////////////////////////////////////////////////
    extern memory_mode mode;

    /// \}
    //////////////////////////////////////////////////////////////////////////////

    /// \}
    //////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Minimum value of 128 MiB for the memory limit.
    ///
    /// \ingroup module__adiar
    //////////////////////////////////////////////////////////////////////////////
    constexpr size_t MINIMUM_BYTES = 128 * 1024 * 1024;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Sets the path for temporary TPIE files.
    //////////////////////////////////////////////////////////////////////////////
    inline void set_path(const std::string &temp_dir = "")
    {
      // Naming scheme and path for temporary files
      tpie::tempname::set_default_base_name("ADIAR");
      tpie::tempname::set_default_extension("adiar");

      if (temp_dir != "") {
        tpie::tempname::set_default_path(temp_dir);
      }
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Sets the memory limit for TPIE.
    ///
    /// \throws std::invalid_argument if the amount of memory given is smaller
    ///         than \ref MINIMUM_BYTES
    //////////////////////////////////////////////////////////////////////////////
    inline void set_limit(size_t memory_limit_bytes = MINIMUM_BYTES)
    {
      if (memory_limit_bytes < MINIMUM_BYTES) {
        throw std::invalid_argument("Adiar requires at least 128 MiB of memory");
      }

      tpie::get_memory_manager().set_limit(memory_limit_bytes);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Minimum block size of 2 MiB.
    //////////////////////////////////////////////////////////////////////////////
    constexpr size_t MINIMUM_BLOCK_SIZE = 2 * 1024 * 1024;

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Sets the block size for TPIE.
    //////////////////////////////////////////////////////////////////////////////
    inline void set_block_size(size_t block_size_bytes)
    {
      tpie::set_block_size(block_size_bytes);
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Get the block size from TPIE.
    //////////////////////////////////////////////////////////////////////////////
    inline size_t get_block_size()
    {
      return tpie::get_block_size();
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Computes a recommended block size to be used with a specific
    ///        amount of available internal memory.
    //////////////////////////////////////////////////////////////////////////////
    inline size_t recommended_block_size(size_t /*memory_limit_bytes*/)
    {
      return MINIMUM_BLOCK_SIZE;
    }

    //////////////////////////////////////////////////////////////////////////////
    /// \brief Obtain from TPIE the amount of available memory.
    //////////////////////////////////////////////////////////////////////////////
    inline size_t available()
    {
      return tpie::get_memory_manager().available();
    }
  }
}

#endif // ADIAR_INTERNAL_MEMORY_H

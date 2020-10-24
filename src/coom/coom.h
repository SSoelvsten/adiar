#ifndef COOM_H
#define COOM_H

// Data structures
#include <coom/data.h>
#include <coom/assignment.h>

// Simple constructors
#include <coom/build.h>

// Algorithms
#include <coom/apply.h>
#include <coom/count.h>
#include <coom/evaluate.h>
#include <coom/negate.h>
#include <coom/reduce.h>
#include <coom/restrict.h>
#include <coom/quantify.h>

// Debugging
#include <coom/dot.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Initiates COOM with the given amount of memory in MB
  ///
  /// TODO: Should we provide an option to change the maximum variable number?
  ///       What about opening files by others? Should we store that somehow in
  ///       the first element of the meta stream?
  //////////////////////////////////////////////////////////////////////////////
  void coom_init(size_t memory_limit_mb);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Changes the memory limit used by COOM
  //////////////////////////////////////////////////////////////////////////////
  void set_limit(size_t memory_limit_mb);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Closes and cleans up everything by COOM
  //////////////////////////////////////////////////////////////////////////////
  void coom_deinit();
}

#endif // COOM_H

#ifndef COOM_H
#define COOM_H

////////////////////////////////////////////////////////////////////////////////
/// COOM Core
#include <coom/data.h>
#include <coom/file.h>
#include <coom/file_stream.h>
#include <coom/file_writer.h>

#include <coom/reduce.h>

////////////////////////////////////////////////////////////////////////////////
/// COOM BDD
#include <coom/bdd/bdd.h>

// Simple constructors
#include <coom/bdd/build.h>

// Algorithms
#include <coom/bdd/apply.h>
#include <coom/bdd/assignment.h>
#include <coom/bdd/count.h>
#include <coom/bdd/evaluate.h>
#include <coom/bdd/negate.h>
#include <coom/bdd/restrict.h>
#include <coom/bdd/quantify.h>

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

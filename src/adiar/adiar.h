/* Adiar is an I/O-efficient implementation of Binary Decision Diagrams (BDDs).
 *
 * Copyright (C) 2021 Steffan Sølvsten
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ADIAR_H
#define ADIAR_H

#include <string>

////////////////////////////////////////////////////////////////////////////////
/// Core
#include <adiar/data.h>
#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

////////////////////////////////////////////////////////////////////////////////
/// BDD
#include <adiar/bdd/bdd.h>

// Simple constructors
#include <adiar/bdd/build.h>

// Manipulation operations
#include <adiar/bdd/apply.h>
#include <adiar/bdd/count.h>
#include <adiar/bdd/if_then_else.h>
#include <adiar/bdd/negate.h>
#include <adiar/bdd/restrict.h>
#include <adiar/bdd/quantify.h>

// Other operations
#include <adiar/bdd/assignment.h>
#include <adiar/bdd/evaluate.h>

////////////////////////////////////////////////////////////////////////////////
/// ZDD
#include <adiar/zdd/zdd.h>

// Simple constructors
#include <adiar/zdd/build.h>

// Manipulation operations
#include <adiar/zdd/binop.h>
#include <adiar/zdd/project.h>

// Other operations
#include <adiar/zdd/count.h>

// Predicates
#include <adiar/zdd/pred.h>

////////////////////////////////////////////////////////////////////////////////
/// Statistics
#include <adiar/statistics.h>

////////////////////////////////////////////////////////////////////////////////
/// Debugging
#include <adiar/dot.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Initiates Adiar with the given amount of memory (given in bytes)
  ///
  /// TODO: Should we provide an option to change the maximum variable number?
  ///       What about opening files by others? Should we store that somehow in
  ///       the first element of the meta stream?
  //////////////////////////////////////////////////////////////////////////////
  void adiar_init(size_t memory_limit_bytes, std::string temp_dir = "");

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Whether Adiar is initialized.
  //////////////////////////////////////////////////////////////////////////////
  bool adiar_initialized();

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Closes and cleans up everything by Adiar
  //////////////////////////////////////////////////////////////////////////////
  void adiar_deinit();
}

#endif // ADIAR_H

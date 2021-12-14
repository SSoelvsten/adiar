/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

#include "evaluate.h"

#include <adiar/file_stream.h>

#include <adiar/assert.h>

namespace adiar
{
  bool bdd_eval(const bdd &bdd, const assignment_file &assignments)
  {
    node_stream<> ns(bdd);
    node_t current_node = ns.pull();

    if(is_sink(current_node)) {
      return value_of(current_node);
    }

    assignment_stream<> as(assignments);
    assignment_t a = as.pull();

    while (true) {
      while(label_of(current_node) > label_of(a)) {
        adiar_assert(as.can_pull(),
                    "Given assignment insufficient to traverse BDD");
        a = as.pull();
      }
      adiar_assert(label_of(current_node) == label_of(a),
                  "Missing assignment for node visited in BDD");

      ptr_t next_ptr = unflag(value_of(a) ? current_node.high : current_node.low);

      if(is_sink(next_ptr)) {
        return value_of(next_ptr);
      }

      while(current_node.uid < next_ptr) {
        adiar_debug(ns.can_pull(), "Invalid uid chasing; fell out of BDD");
        current_node = ns.pull();
      }
    }
  }
}

/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU Lesser General Public License (Free Software
 * Foundation) with a Static Linking Exception.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

#include <adiar/data.h>
#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

namespace adiar
{
  assignment_file assignment_find(const bdd &f,
                                  bool default_for_skipped_var,
                                  const std::function<bool(node_t)>& pick_next)
  {
    adiar_assert(!is_sink(f),
                 "Cannot extract an assignment from a sink file");

    assignment_file out;
    assignment_writer aw(out);

    node_stream<> in_nodes(f);
    node_t v_curr = in_nodes.pull();

    level_info_stream<node_t,1> in_meta(f);

    bool pick_high = pick_next(v_curr);
    aw << create_assignment(label_of(in_meta.pull()), pick_high);

    ptr_t v_next = pick_high ? v_curr.high : v_curr.low;

    while (!is_sink(v_next)) {
      // forward to v_next
      while (v_curr.uid < v_next) { v_curr = in_nodes.pull(); }

      // set default to all skipped levels
      while (label_of(in_meta.peek()) < label_of(v_next)) {
        aw << create_assignment(label_of(in_meta.pull()), default_for_skipped_var);
      }

      pick_high = pick_next(v_curr);
      aw << create_assignment(label_of(in_meta.pull()), pick_high);

      v_next = pick_high ? v_curr.high : v_curr.low;
    }

    // Set the remaining levels to the default value
    while (in_meta.can_pull()) {
      aw << create_assignment(label_of(in_meta.pull()), default_for_skipped_var);
    }
    return out;
  }

  const auto pick_low = [](node_t v) -> bool
  {
    // Only pick high, if low is the false sink
    return is_sink(v.low) && !value_of(v.low);
  };

  assignment_file bdd_satmin(const bdd &f)
  {
    return assignment_find(f, false, pick_low);
  }

  const auto pick_high = [](node_t v) -> bool
  {
    // Pick high as long it is not the false sink
    return is_node(v.high) || value_of(v.high);
  };

  assignment_file bdd_satmax(const bdd &f)
  {
    return assignment_find(f, true, pick_high);
  }
}

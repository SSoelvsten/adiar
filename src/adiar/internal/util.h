/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
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

#ifndef ADIAR_INTERNAL_UTIL_H
#define ADIAR_INTERNAL_UTIL_H

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>

#include <adiar/internal/decision_diagram.h>

namespace adiar {
  template<typename in1_t = decision_diagram, typename stream1_t = level_info_stream<node_t, NODE_FILE_COUNT>,
           typename in2_t = decision_diagram, typename stream2_t = level_info_stream<node_t, NODE_FILE_COUNT>>
  bool disjoint_labels(const in1_t &in1, const in2_t &in2)
  {
    stream1_t s1(in1);
    stream2_t s2(in2);

    while(s1.can_pull() && s2.can_pull()) {
      if (label_of(s1.peek()) == label_of(s2.peek())) {
        return false;
      } else if (label_of(s1.peek()) < label_of(s2.peek())) {
        s1.pull();
      } else {
        s2.pull();
      }
    }
    return true;
  }
}

#endif // ADIAR_INTENRAL_UTIL_H

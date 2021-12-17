////////////////////////////////////////////////////////////////////////////////
///   This Source Code Form is subject to the terms of the Mozilla Public    ///
///   License, v. 2.0. If a copy of the MPL was not distributed with this    ///
///   file, You can obtain one at http://mozilla.org/MPL/2.0/.               ///
////////////////////////////////////////////////////////////////////////////////

#include "levelized_priority_queue.h"

namespace adiar {
  stats_t::priority_queue_t stats_priority_queue;

  tpie::dummy_progress_indicator pq_tpie_progress_indicator { };
}

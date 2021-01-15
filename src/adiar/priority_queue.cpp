#ifndef ADIAR_PRIORITY_QUEUE_CPP
#define ADIAR_PRIORITY_QUEUE_CPP

#include <tpie/priority_queue.h> // imports tpie::consecutive_memory_available

#include "priority_queue.h"

namespace adiar {
  tpie::dummy_progress_indicator pq_tpie_progress_indicator { };

  float calc_tpie_pq_factor(tpie::memory_size_type memory_given)
  {
    // Memory taken by the priority queue, if they could
    size_t mm_avail = tpie::consecutive_memory_available();
    if (mm_avail <= memory_given) { return 1.0; }
    return static_cast<float>(memory_given) / static_cast<float>(mm_avail);
  }
}

#endif // ADIAR_PRIORITY_QUEUE_CPP

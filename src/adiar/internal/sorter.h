#ifndef ADIAR_INTERNAL_SORTER_H
#define ADIAR_INTERNAL_SORTER_H

#include <string>
#include <math.h>

#include <tpie/tpie.h>
#include <tpie/sort.h>

#include <adiar/assert.h>

namespace adiar {
  extern tpie::dummy_progress_indicator external_sorter_tpie_progress_indicator;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Wrapper for TPIE's external memory sorter, tpie::merge_sorter.
  ///
  /// A wrapper for the tpie::merge_sorter that takes care of all the memory
  /// computations involved in deriving how much memory should be used and how
  /// it should be.
  ///
  /// TODO:
  /// - Peek function
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, typename pred_t = std::less<T>>
  class external_sorter {
  private:
    tpie::merge_sorter<T, false, pred_t> _sorter;

  public:
    external_sorter(size_t memory_bytes, size_t number_of_sorters, pred_t pred = pred_t())
      : _sorter(pred)
    {
      adiar_debug(number_of_sorters > 0, "Number of sorters should be positive");
      adiar_debug(number_of_sorters > 1, "TODO: memory calculations for single sorter");

      // We intend to have the memory divided between all the sorters, such that
      // one can be in phase 2 while everyone else is in phase 1 or 3.
      //
      // | p1 | p1 | p1 |       . . . p2 . . .         |
      //
      // Phase two is the one that makes the most out of a lot of memory. So, we
      // want to have phase 2 have the largest share. For simplicity, we
      // currently have all the p1 sorters have 1/16 of the entire memory.
      //
      // TODO: It would be better to make p2 'exponentially' larger than p1 for
      //       some notion of 'exponentiality'.
      //
      // TODO: p2 should be upper-bounded by the maximal number of elements
      //       possible. If this decreases its size compared to what is
      //       above, then the remaining space should be distributed among all
      //       phase 1.

      const tpie::memory_size_type phase1 =
        std::max(std::max(_sorter.minimum_memory_phase_1(), _sorter.minimum_memory_phase_3()),
                 (memory_bytes >> 4) / (number_of_sorters - 1));

      const tpie::memory_size_type phase2 =
        memory_bytes - phase1 * (number_of_sorters - 1);

      const tpie::memory_size_type phase3 =
        phase1;

      // Sanity checks
      adiar_debug(_sorter.minimum_memory_phase_1() <= phase1,
                  "Not enough memory for phase 1");

      adiar_debug(_sorter.minimum_memory_phase_2() <= phase2,
                  "Not enough memory for phase 2");

      adiar_debug(_sorter.minimum_memory_phase_3() <= phase3,
                  "Not enough memory for phase 1");

      // Set the available memory and start the sorter
      _sorter.set_available_memory(phase1, phase2, phase3);
      _sorter.begin();
    }

    void push(const T& t)
    {
      _sorter.push(t);
    }

    void sort()
    {
      _sorter.end();
      _sorter.calc(external_sorter_tpie_progress_indicator);
    }

    bool can_pull()
    {
      return _sorter.can_pull();
    }

    T pull()
    {
      return _sorter.pull();
    }
  };
}

#endif // ADIAR_INTERNAL_SORTER_H

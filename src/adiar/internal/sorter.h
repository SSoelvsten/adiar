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
      // | p1 | p1 | p1 | p3 |                      p2                      |
      //
      // Currently, we have phase 1 and 3 be the smallest they can be (~5 MB).
      // At some point, we would like to make them vary in size, depending on
      // the amount of memory given - assuming this will increase performance on
      // average.
      //
      // TODO: Intuitively 2*p1 <= p3, but the tpie::merge_sorter does not seem
      //       to complain with only p3 = 1*p1. Should we change this?
      //
      // TODO: It would be better to make p2 'exponentially' larger than p1 for
      //       some notion of 'exponentiality'.
      //
      // TODO: phase 1 should be upper bounded by the number of elements
      //       possibly placed in this queue. See Issue #189 on ssoelvsten/adiar
      //       as to why.

      const tpie::memory_size_type phase1 =
        sizeof(T) * 128 * 1024 + 5 * 1024 * 1024;

      const tpie::memory_size_type phase2 =
        memory_bytes - phase1 * (number_of_sorters-1);

      const tpie::memory_size_type phase3 =
        phase1;

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

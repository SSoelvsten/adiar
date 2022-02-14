#ifndef ADIAR_INTERNAL_SORTER_H
#define ADIAR_INTERNAL_SORTER_H

#include <string>
#include <math.h>
#include <algorithm>

#include <tpie/tpie.h>
#include <tpie/sort.h>

#include <adiar/internal/assert.h>

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
    external_sorter(size_t memory_bytes, size_t /*no_elements*/, size_t number_of_sorters, pred_t pred = pred_t())
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
      // TODO: phase 1 should be upper bounded by the number of elements
      //       possibly placed in this queue. See Issue #189 on ssoelvsten/adiar
      //       as to why. I would suggest for to upper bound the 1/16th by
      //       slightly more than the amount of memory necessary to hold all
      //       values simultaneously.

      // Quickfix: Issue #250 of thomasmoelhave/tpie
      const tpie::memory_size_type minimum_phase1 = std::max(_sorter.minimum_memory_phase_1(),
                                                             sizeof(T) * 128 * 1024 + 5 * 1024 * 1024);

      const tpie::memory_size_type phase1 =
        std::max(std::max(minimum_phase1, _sorter.minimum_memory_phase_3()),
                 (memory_bytes >> 4) / (number_of_sorters - 1));

      const tpie::memory_size_type phase2 =
        memory_bytes - phase1 * (number_of_sorters - 1);

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

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Wrapper for TPIE's internal vector with standard quick-sort.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, typename pred_t = std::less<T>>
  class internal_sorter {
  private:
    tpie::internal_vector<T> _vector;
    typename tpie::array<T>::iterator _begin;
    typename tpie::array<T>::iterator _end;
    pred_t _pred;

  public:
    static constexpr tpie::memory_size_type memory_usage(tpie::memory_size_type no_elements)
    {
      return tpie::internal_vector<T>::memory_usage(no_elements);
    }

    static constexpr tpie::memory_size_type memory_fits(tpie::memory_size_type memory_bytes)
    {
      return tpie::internal_vector<T>::memory_fits(memory_bytes);
    }

  public:
    internal_sorter(size_t /*memory_bytes*/, size_t no_elements, size_t /*no_sorters*/, pred_t pred = pred_t())
      : _vector(no_elements), _pred(pred)
    { }

    void push(const T& t)
    {
      _vector.push_back(t);
    }

    void sort()
    {
      tpie::parallel_sort(_vector.begin(), _vector.end(), _pred);
      _begin = _vector.begin();
      _end = _vector.end();
    }

    bool can_pull()
    {
      return _begin != _end;
    }

    T pull()
    {
      const T ret_value = *_begin;
      ++_begin;
      return ret_value;
    }
  };
}

#endif // ADIAR_INTERNAL_SORTER_H

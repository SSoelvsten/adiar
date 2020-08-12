#ifndef COOM_MERGE_SORTER_CPP
#define COOM_MERGE_SORTER_CPP

#include "merge_sorter.h"

namespace coom {
  tpie::dummy_progress_indicator tpie_progress_indicator {};

  template <typename T, typename pred_t>
  void merge_sorter<T, pred_t>::init() {
    _merge_sorter.set_available_memory(tpie::get_memory_manager().available());
    _merge_sorter.begin();
  }

  template <typename T, typename pred_t>
  void merge_sorter<T, pred_t>::init(tpie::memory_size_type memory_limit) {
    _merge_sorter.set_available_memory(memory_limit);
    _merge_sorter.begin();
  }


  template <typename T, typename pred_t>
  void merge_sorter<T, pred_t>::push(const T& item) {
    return _merge_sorter.push(item);
  }

  template <typename T, typename pred_t>
  void merge_sorter<T, pred_t>::sort() {
    _merge_sorter.end();
    _merge_sorter.calc(tpie_progress_indicator);
  }


  template <typename T, typename pred_t>
  bool merge_sorter<T, pred_t>::can_pull() {
    return _merge_sorter.can_pull() || _has_peeked;
  }

  template <typename T, typename pred_t>
  T merge_sorter<T, pred_t>::peek() {
    if (!_has_peeked) {
      _has_peeked = true;
      _peeked = _merge_sorter.pull();
    }
    return _peeked;
  }

  template <typename T, typename pred_t>
  T merge_sorter<T, pred_t>::pull() {
    T retvalue = _has_peeked ? _peeked : _merge_sorter.pull();
    _has_peeked = false;
    return retvalue;
  }
}

#endif // COOM_MERGE_SORTER_CPP

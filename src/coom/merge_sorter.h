#ifndef COOM_MERGE_SORTER_H
#define COOM_MERGE_SORTER_H

#include <type_traits>

#include <tpie/sort.h>

namespace coom {
  template <typename T, typename pred_t = std::less<T>> class merge_sorter
  {
    static_assert(std::is_pod<T>::value, "T must be POD");

  private:
    bool _has_peeked = false;
    T _peeked;

    tpie::merge_sorter<T, false, pred_t> _merge_sorter;

  public:
    merge_sorter() {};
    merge_sorter(const pred_t pred) : _merge_sorter(pred) {};

    void init();
    void init(tpie::memory_size_type memory_limit);

    void push(const T& item);

    void sort();

    bool can_pull();
    T peek();
    T pull();
  };
}

#endif // COOM_MERGE_SORTER_H

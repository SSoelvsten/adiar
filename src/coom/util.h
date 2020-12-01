#ifndef COOM_UTIL_H
#define COOM_UTIL_H

#include <tpie/file.h>
#include <tpie/file_stream.h>

#include <tpie/sort.h>

#include <coom/data.h>
#include <coom/file.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// Copy the content of one file into another.
  ///
  /// \param f1 Copy source
  /// \param f2 Copy target
  //////////////////////////////////////////////////////////////////////////////
  template<typename T>
  void copy(const tpie::file<T> &f1, const tpie::file<T> &f2)
  {
    coom_debug(f2.size() == 0, "Non-empty output file");
    if (f1.size() == 0) { return; }

    typename tpie::file<T>::stream f1_stream(f1);
    typename tpie::file<T>::stream f2_stream(f2);

    while (f1.can_read()) {
      f2_stream.write(f1_stream.read());
    }

    f1_stream.detach();
    f2_stream.detach();
  }

  //////////////////////////////////////////////////////////////////////////////
  /// Copy the content of a merge_sorter into an output file.
  ///
  /// \param sorter The tpie::merge_sorter to copy from. It is assumed, that the
  ///               merge sorter already is in Phase 3, where one may pull.
  ///
  /// \param file_stream The tpie::file_stream to write to.
  //////////////////////////////////////////////////////////////////////////////
  template<typename T, bool UseProgress, typename pred_t = std::less<T>>
  void sort_with_callback(tpie::merge_sorter<T, UseProgress, pred_t> &sorter,
                          std::function<void(const T&)> callback,
                          bool run_sorter = true)
  {
    if (run_sorter) {
      sorter.end();
      if constexpr (UseProgress) {
        tpie::progress_indicator_null pi;
        sorter.calc(pi);
      } else {
        tpie::dummy_progress_indicator pi;
        sorter.calc(pi);
      }
    }

    if (!sorter.can_pull()) { return; }

    while (sorter.can_pull()) {
      T t = sorter.pull();
      callback(t);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  /// A non-persistable external memory FIFO queue as shown in the TPIE
  /// documentation.
  ///
  /// https://users-cs.au.dk/rav/tpie/doc/master/queue.html
  //////////////////////////////////////////////////////////////////////////////
  template<typename T>
  class fifo_queue
  {
  private:
    tpie::file<T> queue;

    typename tpie::file<T>::stream front;
    typename tpie::file<T>::stream back;

    tpie::stream_size_type enqueued = 0;

  public:
    fifo_queue()
    {
      queue.open();
      front.attach(queue);
      back.attach(queue);
    }

    ~fifo_queue()
    {
      front.detach();
      back.detach();
      queue.close();
    }

    void enqueue(const T& t)
    {
      back.write(t);
      ++enqueued;
    }

    T dequeue()
    {
      coom_debug(!empty(), "Dequeue on empty queue");

      --enqueued;
      return front.read();
    }

    bool empty()
    {
      return enqueued == 0;
    }
  };
}

#endif // COOM_UTIL_H

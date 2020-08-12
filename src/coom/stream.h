#ifndef COOM_STREAM_H
#define COOM_STREAM_H

#include <type_traits>
#include <memory>

#include <tpie/file_stream.h>

namespace coom {
  template <typename T> class out_stream;

  //////////////////////////////////////////////////////////////////////////////
  /// Reading direction policies
  //////////////////////////////////////////////////////////////////////////////
  template <typename T> class forward
  {
  protected:
    bool can_pull(std::shared_ptr<tpie::file_stream<T>>&);
    T peek(std::shared_ptr<tpie::file_stream<T>>&);
    T pull(std::shared_ptr<tpie::file_stream<T>>&);

    void reset(std::shared_ptr<tpie::file_stream<T>>&);
  };

  template <typename T> class reverse
  {
  private:
    bool _has_next = false;
    T _next;

  protected:
    bool can_pull(std::shared_ptr<tpie::file_stream<T>>&);
    T peek(std::shared_ptr<tpie::file_stream<T>>&);
    T pull(std::shared_ptr<tpie::file_stream<T>>&);

    void reset(std::shared_ptr<tpie::file_stream<T>>&);
  };

  //////////////////////////////////////////////////////////////////////////////
  /// Negation policy (dummy)
  //////////////////////////////////////////////////////////////////////////////
  template <typename T> class negator {
  protected:
    T negate(const T& x) { return x; }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Generic read only stream
  ///
  /// Decorator for the tpie::file_stream that provides an O(1) negation mapping
  /// function for each read.
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, typename direction=forward<T>, typename negator=negator<T>>
  class in_stream : private direction, private negator
  {
    static_assert(std::is_pod<T>::value, "T must be POD");

  private:
    bool _use_negate = false;

  public:
    std::shared_ptr<tpie::file_stream<T>> _tpie_stream_ptr;

    in_stream(std::shared_ptr<tpie::file_stream<T>>& tpie_stream_ptr);
    in_stream(const out_stream<T>& os);
    in_stream(const std::string& path);

    void negate();

    bool can_pull();
    T peek();
    T pull();

    void reset();

  private:
    T negate(const T& x);

    using direction::can_pull;
    using direction::peek;
    using direction::pull;

    using direction::reset;

    using negator::negate;
  };


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Generic write-only stream
  //////////////////////////////////////////////////////////////////////////////
  template <typename T> class out_stream {
    static_assert(std::is_pod<T>::value, "T must be POD");

  public:
    std::shared_ptr<tpie::file_stream<T>> _tpie_stream_ptr;

    out_stream();
    out_stream(const std::string &path);

    void push(const T& item);

    bool is_empty();
  };

  template <typename T> void operator << (out_stream<T>& os, const T& item)
  {
    os.push(item);
  }
}

#endif // COOM_STREAM_H

#ifndef COOM_STREAM_CPP
#define COOM_STREAM_CPP

#include <tpie/file_stream.h>

#include "stream.h"

namespace coom {
  //////////////////////////////////////////////////////////////////////////////
  /// Generic read only stream
  template <typename T, typename direction, typename negator>
  in_stream<T, direction, negator>::in_stream(std::shared_ptr<tpie::file_stream<T>>& tpie_stream_ptr)
  {
    _tpie_stream_ptr = tpie_stream_ptr;
  }

  template <typename T, typename direction, typename negator>
  in_stream<T, direction, negator>::in_stream(const out_stream<T>& os)
  {
    _tpie_stream_ptr = os._tpie_stream_ptr;
  }

  template <typename T, typename direction, typename negator>
  in_stream<T, direction, negator>::in_stream(const std::string& path) {
    _tpie_stream_ptr = std::make_shared<tpie::file_stream<T>>();
    _tpie_stream_ptr -> open(path, tpie::open::read_only | tpie::open::compression_normal);
  };


  template <typename T, typename direction, typename negator>
  void in_stream<T, direction, negator>::negate()
  { _use_negate = !_use_negate; }

  template <typename T, typename direction, typename negator>
  bool in_stream<T, direction, negator>::can_pull()
  {
    return direction::can_pull(_tpie_stream_ptr);
  }

  template <typename T, typename direction, typename negator>
  T in_stream<T, direction, negator>::peek()
  {
    return _use_negate
      ? negator::negate(direction::peek(_tpie_stream_ptr))
      : direction::peek(_tpie_stream_ptr);
  }

  template <typename T, typename direction, typename negator>
  T in_stream<T, direction, negator>::pull()
  {
    return _use_negate
      ? negator::negate(direction::pull(_tpie_stream_ptr))
      : direction::pull(_tpie_stream_ptr);
  }

  template <typename T, typename direction, typename negator>
  void in_stream<T, direction, negator>::reset()
  {
    direction::reset(_tpie_stream_ptr);
  }


  //////////////////////////////////////////////////////////////////////////////
  /// Normal forward direction policy
  template <typename T>
  bool forward<T>::can_pull(std::shared_ptr<tpie::file_stream<T>>& _tpie_stream_ptr)
  {
    return _tpie_stream_ptr -> can_read();
  }

  template <typename T>
  T forward<T>::peek(std::shared_ptr<tpie::file_stream<T>>& _tpie_stream_ptr)
  {
    return _tpie_stream_ptr -> peek();
  }

  template <typename T>
  T forward<T>::pull(std::shared_ptr<tpie::file_stream<T>>& _tpie_stream_ptr)
  {
    return _tpie_stream_ptr -> read();
  }

  template <typename T>
  void forward<T>::reset(std::shared_ptr<tpie::file_stream<T>>& _tpie_stream_ptr)
  {
    if (_tpie_stream_ptr -> can_read_back()) {
      return _tpie_stream_ptr -> seek(0);
    }
  }


  //////////////////////////////////////////////////////////////////////////////
  /// Reversed direction policy
  template <typename T>
  bool reverse<T>::can_pull(std::shared_ptr<tpie::file_stream<T>>& _tpie_stream_ptr)
  { return _tpie_stream_ptr -> can_read_back() || _has_next; }

  template <typename T>
  T reverse<T>::peek(std::shared_ptr<tpie::file_stream<T>>& _tpie_stream_ptr)
  {
    if (!_has_next) {
      _next = _tpie_stream_ptr -> read_back();
      _has_next = true;
    }
    return _next;
  }

  template <typename T>
  T reverse<T>::pull(std::shared_ptr<tpie::file_stream<T>>& _tpie_stream_ptr)
  {
    T retvalue = _has_next ? _next : _tpie_stream_ptr -> read_back();
    _has_next = false;
    return retvalue;
  }

  template <typename T>
  void reverse<T>::reset(std::shared_ptr<tpie::file_stream<T>>& _tpie_stream_ptr)
  {
    _has_next = false;
    if (_tpie_stream_ptr -> can_read()) {
      return _tpie_stream_ptr -> seek(0, tpie::file_stream_base::end);
    }
  }


  //////////////////////////////////////////////////////////////////////////////
  /// Generic write only stream
  template <typename T> out_stream<T>::out_stream() {
    _tpie_stream_ptr = std::make_shared<tpie::file_stream<T>>();
    _tpie_stream_ptr -> open(tpie::open::compression_normal);
  };

  template <typename T> out_stream<T>::out_stream(const std::string& path) {
    _tpie_stream_ptr = std::make_shared<tpie::file_stream<T>>();
    _tpie_stream_ptr -> open(path, tpie::open::compression_normal);
  };

  template <typename T> void out_stream<T>::push(const T& item)
  {
    _tpie_stream_ptr -> write(item);
  }

  template <typename T> bool out_stream<T>::is_empty()
  {
    return (_tpie_stream_ptr -> size()) > 0;
  }
}

#endif // COOM_STREAM_CPP

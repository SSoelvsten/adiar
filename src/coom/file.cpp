#ifndef COOM_FILE_CPP
#define COOM_FILE_CPP

#include "file.h"

#include <coom/assert.h>
#include <coom/reduce.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// node_file
  node_file::node_file(const node_or_arc_file &other) : file(reduce(other)) { }

  node_file& node_file::operator= (const node_or_arc_file &other)
  {
    node_file reduced = reduce(other);
    this -> _meta_file = reduced._meta_file;
    this -> _files[0] = reduced._files[0];

    return *this;
  }

  bool is_sink(const node_file &file, const sink_pred &pred)
  {
    if (file.size() != 1) {
      return false;
    }
    node_stream ns(file);
    node_t n = ns.pull();

    return is_sink(n) && pred(n.uid);
  }

  node_t root_of(const node_file &file)
  {
    node_stream ns(file);
    return ns.pull();
  }

  template<bool from_start>
  label_t extract_label(const node_file &file)
  {
    coom_assert(file.size() > 0, "Cannot extract label from an empty file");

    file_stream<node_t, from_start> ns(file._files[0]);
    node_t n = ns.pull();

    coom_assert(!is_sink(n), "Cannot extract label from sink-only file");

    return label_of(n);
  }

  label_t min_label(const node_file &file)
  {
    return extract_label<true>(file);
  }

  label_t max_label(const node_file &file)
  {
    return extract_label<false>(file);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// node_writer
  node_writer& operator<< (node_writer &nw, const node_t &n)
  {
    nw.push(n);
    return nw;
  }
}

#endif // COOM_FILE_CPP

#ifndef COOM_FILE_CPP
#define COOM_FILE_CPP

#include "file.h"

#include <coom/assert.h>
#include <coom/reduce.h>

namespace coom
{
  //////////////////////////////////////////////////////////////////////////////
  /// node_file
  node_file::node_file(const node_or_arc_file &other) : node_file(reduce(other)) { }

  node_file& node_file::operator= (const node_or_arc_file &other)
  {
    // Clean up current node_file before reduce
    this -> _file_ptr.reset();

    // Reduce and copy over pointer to result
    node_file reduced = reduce(other);
    this -> _file_ptr = reduced._file_ptr;

    // Return this object
    return *this;
  }

  bool is_sink(const node_file &file, const sink_pred &pred)
  {
    if (file.size() != 1) {
      return false;
    }
    node_stream<> ns(file);
    node_t n = ns.pull();

    return is_sink(n) && pred(n.uid);
  }

  node_t root_of(const node_file &file)
  {
    node_stream<> ns(file);
    return ns.pull();
  }

  template<bool reverse>
  label_t extract_label(const node_file &file)
  {
    coom_assert(file.size() > 0, "Cannot extract label from an empty file");

    node_stream<reverse> ns(file);
    node_t n = ns.pull();

    coom_assert(!is_sink(n), "Cannot extract label from sink-only file");

    return label_of(n);
  }

  label_t min_label(const node_file &file)
  {
    return extract_label<false>(file);
  }

  label_t max_label(const node_file &file)
  {
    return extract_label<true>(file);
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

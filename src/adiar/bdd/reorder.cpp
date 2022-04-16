#include <adiar/bdd.h>
#include <adiar/bdd/bdd_policy.h>

#include <adiar/data.h>

#include <adiar/file.h>
#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/substitution.h>
#include <adiar/internal/util.h>

namespace adiar
{
  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_reorder(const bdd &dd)
  {
    return bdd_sink(false);
  }

  std::vector<label_t> bdd_order(const bdd &dd)
  {
    std::vector<label_t> result = std::vector<label_t>();
    label_file labels = bdd_varprofile(dd);
    adiar::file_stream<label_t> fs(labels);

    while (fs.can_pull())
    {
      label_t l = fs.pull();
      result.push_back(l);
    }

    fs.detach();
    return result;
  }
}

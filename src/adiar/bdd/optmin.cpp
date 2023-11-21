#include "adiar/exec_policy.h"
#include "adiar/internal/io/node_writer.h"
#include "adiar/internal/io/shared_file_ptr.h"
#include "adiar/types.h"
#include <adiar/bdd.h>

#include <adiar/bdd/bdd_policy.h>

namespace adiar
{

double
bdd_optmin(const exec_policy &ep,
           const bdd &f,
           const cost<bdd::label_type> &c,
           const consumer<bdd::label_type, bool> &cb)
{
    // TODO!: Call internal::optmin??
}

double
bdd_optmin(const bdd &f,
           const cost<bdd::label_type> &c,
           const consumer<bdd::label_type, bool> &cb)
{
    return bdd_optmin(exec_policy(), f, c, cb);
}

pair<bdd, double>
bdd_optmin(const exec_policy &ep,
           const bdd &f,
           const cost<bdd::label_type> &c)
{
    bdd::shared_node_file_type nf;
    internal::node_writer nw(nf);
    bdd::pointer_type root = bdd::pointer_type(true);
    double value = bdd_optmin(ep, f, c, [&nw, &root](bdd::label_type lt, bool value) {
        bdd::node_type next(lt, bdd::max_id, value ? bdd::pointer_type(false) : root, value ? root : bdd::pointer_type(false));
        nw<<next;
        root = next.uid();
    });
    return {nf, value};
}

pair<bdd, double>
bdd_optmin(const bdd &f, const cost<bdd::label_type> &c)
{
    return bdd_optmin(exec_policy(), f, c);
}
}

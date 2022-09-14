#include <adiar/bdd.h>

#include <adiar/internal/algorithms/pred.h>

namespace adiar
{
    bool bdd_equal(const bdd &f, const bdd &g)
    {
        return is_isomorphic(f, g);
    }
}

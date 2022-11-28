#include "test.h"

////////////////////////////////////////////////////////////////////////////////
// ADIAR Core unit tests
#include "adiar/test_assignment.cpp"
#include "adiar/test_bool_op.cpp"

#include "adiar/test_domain.cpp"
#include "adiar/test_builder.cpp"

////////////////////////////////////////////////////////////////////////////////
// ADIAR Internal unit tests
#include "adiar/internal/data_types/test_ptr.cpp"
#include "adiar/internal/data_types/test_uid.cpp"
#include "adiar/internal/data_types/test_node.cpp"
#include "adiar/internal/data_types/test_arc.cpp"
#include "adiar/internal/data_types/test_tuple.cpp"
#include "adiar/internal/data_types/test_convert.cpp"

#include "adiar/internal/io/test_file.cpp"
#include "adiar/internal/io/test_levelized_file.cpp"

#include "adiar/internal/data_structures/test_levelized_priority_queue.cpp"

#include "adiar/internal/test_dot.cpp"
#include "adiar/internal/test_util.cpp"

#include "adiar/internal/algorithms/test_convert.cpp"
#include "adiar/internal/algorithms/test_isomorphism.cpp"
#include "adiar/internal/algorithms/test_reduce.cpp"

////////////////////////////////////////////////////////////////////////////////
// ADIAR BDD unit tests
#include "adiar/bdd/test_bdd.cpp"

#include "adiar/bdd/test_apply.cpp"
#include "adiar/bdd/test_assignment.cpp"
#include "adiar/bdd/test_build.cpp"
#include "adiar/bdd/test_count.cpp"
#include "adiar/bdd/test_evaluate.cpp"
#include "adiar/bdd/test_if_then_else.cpp"
#include "adiar/bdd/test_negate.cpp"
#include "adiar/bdd/test_quantify.cpp"
#include "adiar/bdd/test_restrict.cpp"

////////////////////////////////////////////////////////////////////////////////
// ADIAR ZDD unit tests
#include "adiar/zdd/test_zdd.cpp"

#include "adiar/zdd/test_binop.cpp"
#include "adiar/zdd/test_build.cpp"
#include "adiar/zdd/test_change.cpp"
#include "adiar/zdd/test_complement.cpp"
#include "adiar/zdd/test_contains.cpp"
#include "adiar/zdd/test_count.cpp"
#include "adiar/zdd/test_elem.cpp"
#include "adiar/zdd/test_expand.cpp"
#include "adiar/zdd/test_pred.cpp"
#include "adiar/zdd/test_project.cpp"
#include "adiar/zdd/test_subset.cpp"

#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/negate.cpp", []() {
    shared_levelized_file<bdd::node_type> terminal_T_nf;

    { // Garbage collect writer to free write-lock
      node_writer nw(terminal_T_nf);
      nw << node(true);
    }

    bdd terminal_T(terminal_T_nf);

    shared_levelized_file<bdd::node_type> terminal_F_nf;

    { // Garbage collect writer to free write-lock
      node_writer nw(terminal_F_nf);
      nw << node(false);
    }

    bdd terminal_F(terminal_F_nf);

    shared_levelized_file<bdd::node_type> bdd_1_nf;
    /*
          1      ---- x0
         / \
         | 2     ---- x1
         |/ \
         3  |    ---- x2
         |\ /
         F T
    */

    ptr_uint64 terminal_T_ptr = ptr_uint64(true);
    ptr_uint64 terminal_F_ptr = ptr_uint64(false);

    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_1_nf);

      nw << node(2, node::max_id, terminal_F_ptr, terminal_T_ptr)
         << node(1, node::max_id, ptr_uint64(2, ptr_uint64::max_id), terminal_T_ptr)
         << node(0, node::max_id, ptr_uint64(2, ptr_uint64::max_id), ptr_uint64(1, ptr_uint64::max_id));
    }

    bdd bdd_1(bdd_1_nf);

    shared_levelized_file<bdd::node_type> bdd_2_nf;
    /*
           1     ---- x0
          / \
         2   3   ---- x1
         |\ /|
          \T/
           6     ---- x2
          / \
          F T
    */

    { // Garbage collect writer to free write-lock
      node_writer nw(bdd_2_nf);

      nw << node(2, node::max_id, terminal_F_ptr, terminal_T_ptr)
         << node(1, node::max_id, ptr_uint64(2, ptr_uint64::max_id), terminal_T_ptr)
         << node(1, node::max_id-1, terminal_T_ptr, ptr_uint64(2, ptr_uint64::max_id))
         << node(0, node::max_id, ptr_uint64(1, ptr_uint64::max_id-1), ptr_uint64(1, ptr_uint64::max_id));
    }

    bdd bdd_2(bdd_2_nf);

    it("should doube-negate a T terminal-only BDD back into a T terminal-only BDD", [&]() {
      bdd out = bdd_not(bdd_not(terminal_T));

      // Check if it is correct
      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(node(true)));
      AssertThat(ns.can_pull(), Is().False());
    });

    it("should doube-negate an F terminal-only BDD back into an F terminal-only BDD", [&]() {
      bdd out = bdd_not(bdd_not(terminal_T));

      // Check if it is correct
      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(node(true)));
      AssertThat(ns.can_pull(), Is().False());
    });

    it("should double-negate terminal-children in BDD 1 back to the original", [&]() {
      // Checkmate, constructivists...

      bdd out = bdd_not(bdd_not(bdd_1));

      // Check if it is correct
      node_test_stream ns(out);

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(node(2, node::max_id,
                                                     terminal_F_ptr,
                                                     terminal_T_ptr)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(node(1, node::max_id,
                                                     ptr_uint64(2, ptr_uint64::max_id),
                                                     terminal_T_ptr)));

      AssertThat(ns.can_pull(), Is().True());
      AssertThat(ns.pull(), Is().EqualTo(node(0, node::max_id,
                                                     ptr_uint64(2, ptr_uint64::max_id),
                                                     ptr_uint64(1, ptr_uint64::max_id))));

      AssertThat(ns.can_pull(), Is().False());
    });

    describe("bdd_not(const &)", [&]() {
      it("should negate a T terminal-only BDD into an F terminal-only BDD", [&]() {
        bdd out = bdd_not(terminal_T);

        // Check if it is correct
        node_test_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());
      });

      it("should negate a F terminal-only BDD into an T terminal-only BDD", [&]() {
        bdd out = bdd_not(terminal_F);

        // Check if it is correct
        node_test_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());
      });

      it("should negate terminal-children in BDD 1", [&]() {
        bdd out = bdd_not(bdd_1);

        // Check if it is correct
        node_test_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::max_id,
                                                       terminal_T_ptr,
                                                       terminal_F_ptr)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::max_id,
                                                       ptr_uint64(2, ptr_uint64::max_id),
                                                       terminal_F_ptr)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, node::max_id,
                                                       ptr_uint64(2, ptr_uint64::max_id),
                                                       ptr_uint64(1, ptr_uint64::max_id))));

        AssertThat(ns.can_pull(), Is().False());
      });

      it("should double-negate terminal-children in BDD 1 back to the original", [&]() {
        bdd temp = bdd_not(bdd_1);
        bdd out = bdd_not(temp);

        // Check if it is correct
        node_test_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::max_id,
                                                       terminal_F_ptr,
                                                       terminal_T_ptr)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::max_id,
                                                       ptr_uint64(2, ptr_uint64::max_id),
                                                       terminal_T_ptr)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, node::max_id,
                                                       ptr_uint64(2, ptr_uint64::max_id),
                                                       ptr_uint64(1, ptr_uint64::max_id))));

        AssertThat(ns.can_pull(), Is().False());
      });

      it("should negate terminal-children in BDD 2", [&]() {
        bdd out = bdd_not(bdd_2_nf);

        node_test_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::max_id,
                                                       terminal_T_ptr,
                                                       terminal_F_ptr)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::max_id,
                                                       ptr_uint64(2, ptr_uint64::max_id),
                                                       terminal_F_ptr)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::max_id-1,
                                                       terminal_F_ptr,
                                                       ptr_uint64(2, ptr_uint64::max_id))));


        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, node::max_id,
                                                       ptr_uint64(1, ptr_uint64::max_id-1),
                                                       ptr_uint64(1, ptr_uint64::max_id))));

        AssertThat(ns.can_pull(), Is().False());
      });
    });

    describe("bdd_not(&&)", [&]() {
      it("should negate a T terminal-only BDD into an F terminal-only BDD", [&]() {
        bdd out = bdd_not(terminal_T_nf);

        // Check if it is correct
        node_test_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(false)));
        AssertThat(ns.can_pull(), Is().False());
      });

      it("should negate a F terminal-only BDD into an T terminal-only BDD", [&]() {
        bdd out = bdd_not(terminal_F);

        // Check if it is correct
        node_test_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(true)));
        AssertThat(ns.can_pull(), Is().False());
      });

      it("should negate terminal-children in BDD 1", [&]() {
        bdd out = bdd_not(bdd_1_nf);

        // Check if it is correct
        node_test_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::max_id,
                                                       terminal_T_ptr,
                                                       terminal_F_ptr)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::max_id,
                                                       ptr_uint64(2, ptr_uint64::max_id),
                                                       terminal_F_ptr)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, node::max_id,
                                                       ptr_uint64(2, ptr_uint64::max_id),
                                                       ptr_uint64(1, ptr_uint64::max_id))));

        AssertThat(ns.can_pull(), Is().False());
      });

      it("should negate terminal-children in BDD 2", [&]() {
        bdd out = bdd_not(bdd_2_nf);

        node_test_stream ns(out);

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(2, node::max_id,
                                                       terminal_T_ptr,
                                                       terminal_F_ptr)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::max_id,
                                                       ptr_uint64(2, ptr_uint64::max_id),
                                                       terminal_F_ptr)));

        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(1, node::max_id-1,
                                                       terminal_F_ptr,
                                                       ptr_uint64(2, ptr_uint64::max_id))));


        AssertThat(ns.can_pull(), Is().True());
        AssertThat(ns.pull(), Is().EqualTo(node(0, node::max_id,
                                                       ptr_uint64(1, ptr_uint64::max_id-1),
                                                       ptr_uint64(1, ptr_uint64::max_id))));

        AssertThat(ns.can_pull(), Is().False());
      });
    });
  });
 });

#include "../../test.h"

go_bandit([]() {
  describe("adiar/bdd/assignment.cpp", []() {
    ptr_uint64 terminal_T = ptr_uint64(true);
    ptr_uint64 terminal_F = ptr_uint64(false);

    shared_levelized_file<bdd::node_t> bdd_1;
    /*
              1      ---- x0
             / \
             2 |     ---- x1
            / \|
           3   4     ---- x2
          / \ / \
          F 5 T F    ---- x3
           / \
           F T
    */

    {
      node n5 = node(3,0, terminal_F, terminal_T);
      node n4 = node(2,1, terminal_T, terminal_F);
      node n3 = node(2,0, terminal_F, n5.uid());
      node n2 = node(1,0, n3.uid(), n4.uid());
      node n1 = node(0,0, n2.uid(), n4.uid());

      node_writer nw(bdd_1);
      nw << n5 << n4 << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_t> bdd_2;
    /*
               1       ---- x0
              / \
             2   \     ---- x1
            / \   \
           3   4   5   ---- x2
          / \ / \ / \
          F | T F | T  ---- x3
            \__ __/
               6
              / \
              T F
    */

    { // Garbage collect writer to free write-lock
      node n6 = node(3,0, terminal_T, terminal_F);
      node n5 = node(2,2, n6.uid(), terminal_T);
      node n4 = node(2,1, terminal_T, terminal_F);
      node n3 = node(2,0, terminal_F, n6.uid());
      node n2 = node(1,0, n3.uid(), n4.uid());
      node n1 = node(0,0, n2.uid(), n5.uid());

      node_writer nw(bdd_2);
      nw << n6 << n5 << n4 << n3 << n2 << n1;
    }

    shared_levelized_file<bdd::node_t> bdd_3;
    /*

                      1     ---- x1
                     / \
                     2  \   ---- x3
                    / \ /
                   3   4    ---- x5
                  / \ / \
                  T F F T
    */

    { // Garbage collect writer to free write-lock
      node n4 = node(5,1, terminal_F, terminal_T);
      node n3 = node(5,0, terminal_T, terminal_F);
      node n2 = node(3,0, n3.uid(), n4.uid());
      node n1 = node(1,0, n2.uid(), n4.uid());

      node_writer nw(bdd_3);
      nw << n4 << n3 << n2 << n1;
    }

    describe("bdd_satmin(f)", [&]() {
      it("should retrieve assignment from true terminal", [&]() {
        shared_levelized_file<bdd::node_t> T;
        {
          node_writer nw(T);
          nw << node(true);
        }

        adiar::shared_file<assignment_t> result = bdd_satmin(T);

         adiar::file_stream<assignment_t> out_assignment(result);
        AssertThat(out_assignment.can_pull(), Is().False());
      });

      it("should retrieve assignment from false terminal", [&]() {
        shared_levelized_file<bdd::node_t> F;
        {
          node_writer nw(F);
          nw << node(false);
        }

        adiar::shared_file<assignment_t> result = bdd_satmin(false);

         adiar::file_stream<assignment_t> out_assignment(result);
        AssertThat(out_assignment.can_pull(), Is().False());
      });

      it("should retrieve assignment [1]", [&]() {
        adiar::shared_file<assignment_t> result = bdd_satmin(bdd_1);
         adiar::file_stream<assignment_t> out_assignment(result);

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(0, false)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(1, false)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(2, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(3, true)));

        AssertThat(out_assignment.can_pull(), Is().False());
      });

      it("should retrieve assignment [1]", [&]() {
        adiar::shared_file<assignment_t> result = bdd_satmin(bdd_1);
         adiar::file_stream<assignment_t> out_assignment(result);

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(0, false)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(1, false)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(2, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(3, true)));

        AssertThat(out_assignment.can_pull(), Is().False());
      });

      it("should retrieve assignment [~1]", [&]() {
        adiar::shared_file<assignment_t> result = bdd_satmin(bdd_not(bdd_1));
         adiar::file_stream<assignment_t> out_assignment(result);

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(0, false)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(1, false)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(2, false)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(3, false)));

        AssertThat(out_assignment.can_pull(), Is().False());
      });

      it("should retrieve assignment [2]", [&]() {
        adiar::shared_file<assignment_t> result = bdd_satmin(bdd_2);
         adiar::file_stream<assignment_t> out_assignment(result);

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(0, false)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(1, false)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(2, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(3, false)));

        AssertThat(out_assignment.can_pull(), Is().False());
      });

      it("should retrieve assignment [3]", [&]() {
        adiar::shared_file<assignment_t> result = bdd_satmin(bdd_3);
         adiar::file_stream<assignment_t> out_assignment(result);

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(1, false)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(3, false)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(5, false)));

        AssertThat(out_assignment.can_pull(), Is().False());
      });

      it("should retrieve assignment [3]", [&]() {
        adiar::shared_file<assignment_t> result = bdd_satmin(bdd_not(bdd_3));
         adiar::file_stream<assignment_t> out_assignment(result);

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(1, false)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(3, false)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(5, true)));

        AssertThat(out_assignment.can_pull(), Is().False());
      });
    });

    describe("bdd_satmax", [&]() {
      it("should retrieve maximal assignment [1]", [&]() {
        adiar::shared_file<assignment_t> result = bdd_satmax(bdd_1);
         adiar::file_stream<assignment_t> out_assignment(result);

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(0, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(1, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(2, false)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(3, true)));

        AssertThat(out_assignment.can_pull(), Is().False());
      });

      it("should retrieve maximal assignment [~1]", [&]() {
        adiar::shared_file<assignment_t> result = bdd_satmax(bdd_not(bdd_1));
         adiar::file_stream<assignment_t> out_assignment(result);

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(0, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(1, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(2, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(3, true)));

        AssertThat(out_assignment.can_pull(), Is().False());
      });

      it("should retrieve maximal assignment [2]", [&]() {
        adiar::shared_file<assignment_t> result = bdd_satmax(bdd_2);
         adiar::file_stream<assignment_t> out_assignment(result);

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(0, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(1, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(2, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(3, true)));

        AssertThat(out_assignment.can_pull(), Is().False());
      });

      it("should retrieve maximal assignment [~2]", [&]() {
        adiar::shared_file<assignment_t> result = bdd_satmax(bdd_not(bdd_2));
         adiar::file_stream<assignment_t> out_assignment(result);

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(0, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(1, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(2, false)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(3, true)));

        AssertThat(out_assignment.can_pull(), Is().False());
      });

      it("should retrieve maximal assignment [3]", [&]() {
        adiar::shared_file<assignment_t> result = bdd_satmax(bdd_3);
         adiar::file_stream<assignment_t> out_assignment(result);

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(1, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(3, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(5, true)));

        AssertThat(out_assignment.can_pull(), Is().False());
      });

      it("should retrieve maximal assignment [3]", [&]() {
        adiar::shared_file<assignment_t> result = bdd_satmax(bdd_not(bdd_3));
         adiar::file_stream<assignment_t> out_assignment(result);

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(1, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(3, true)));

        AssertThat(out_assignment.can_pull(), Is().True());
        AssertThat(out_assignment.pull(), Is().EqualTo(create_assignment(5, false)));

        AssertThat(out_assignment.can_pull(), Is().False());
      });
    });
  });
 });

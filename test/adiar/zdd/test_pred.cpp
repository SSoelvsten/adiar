go_bandit([]() {
    describe("ZDD: Predicates", [&]() {
        ptr_t sink_T = create_sink_ptr(true);
        ptr_t sink_F = create_sink_ptr(false);

        // Ø
        node_file zdd_empty_nf;
        { node_writer nw(zdd_empty_nf);
          nw << create_sink(false);
        }

        node_file zdd_empty_nf_copy;
        { node_writer nw(zdd_empty_nf_copy);
          nw << create_sink(false);
        }

        // { Ø }
        node_file zdd_null_nf;
        { node_writer nw(zdd_null_nf);
          nw << create_sink(true);
        }

        node_file zdd_null_nf_copy;
        { node_writer nw(zdd_null_nf_copy);
          nw << create_sink(true);
        }

        // { {1} }
        node_file zdd_A_nf;
        { node_writer nw(zdd_A_nf);
          nw << create_node(1,MAX_ID, sink_F, sink_T);
        }

        node_file zdd_A_nf_copy;
        { node_writer nw(zdd_A_nf_copy);
          nw << create_node(1,MAX_ID, sink_F, sink_T);
        }

        // { {1}, {1,2} }
        node_file zdd_B_nf;
        { node_writer nw(zdd_B_nf);
          nw << create_node(2,MAX_ID, sink_T, sink_T)
             << create_node(1,MAX_ID, sink_F, create_node_ptr(2,MAX_ID))
            ;
        }

        // { Ø, {1}, {2}, {1,2} }
        node_file zdd_C_nf;
        { node_writer nw(zdd_C_nf);
          nw << create_node(2,MAX_ID, sink_T,                    sink_T)
             << create_node(1,MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(2,MAX_ID))
            ;
        }

        node_file zdd_C_nf_copy;
        { node_writer nw(zdd_C_nf_copy);
          nw << create_node(2,MAX_ID, sink_T,                    sink_T)
             << create_node(1,MAX_ID, create_node_ptr(2,MAX_ID), create_node_ptr(2,MAX_ID))
            ;
        }

        // { {1}, {1,2}, {0,1,3} }
        node_file zdd_D_nf;
        { node_writer nw(zdd_D_nf);
          nw << create_node(3,MAX_ID,   sink_F,                      sink_T)
             << create_node(2,MAX_ID,   sink_T,                      sink_T)
             << create_node(1,MAX_ID,   sink_F,                      create_node_ptr(3,MAX_ID))
             << create_node(1,MAX_ID-1, sink_F,                      create_node_ptr(2,MAX_ID))
             << create_node(0,MAX_ID,   create_node_ptr(1,MAX_ID-1), create_node_ptr(1,MAX_ID))
            ;
        }

        node_file zdd_D_nf_copy;
        { node_writer nw(zdd_D_nf_copy);
          nw << create_node(3,MAX_ID,   sink_F,                      sink_T)
             << create_node(2,MAX_ID,   sink_T,                      sink_T)
             << create_node(1,MAX_ID,   sink_F,                      create_node_ptr(3,MAX_ID))
             << create_node(1,MAX_ID-1, sink_F,                      create_node_ptr(2,MAX_ID))
             << create_node(0,MAX_ID,   create_node_ptr(1,MAX_ID-1), create_node_ptr(1,MAX_ID))
            ;
        }

        // { Ø, {0,3} }
        node_file zdd_E_nf;
        { node_writer nw(zdd_E_nf);
          nw << create_node(3,MAX_ID, sink_F, sink_T)
             << create_node(0,MAX_ID, sink_T, create_node_ptr(3,MAX_ID))
            ;
        }

        node_file zdd_E_nf_copy;
        { node_writer nw(zdd_E_nf_copy);
          nw << create_node(3,MAX_ID, sink_F, sink_T)
             << create_node(0,MAX_ID, sink_T, create_node_ptr(3,MAX_ID))
            ;
        }

        // { {2} }
        node_file zdd_F_nf;
        { node_writer nw(zdd_F_nf);
          nw << create_node(2,MAX_ID, sink_F, sink_T);
        }

        // Equal and unequal will not be tested, since it merely is a call to
        // is_isomorphic.

        describe("zdd_subseteq", [&]() {
            it("accepts same file", [&]() {
                AssertThat(zdd_subseteq(zdd_A_nf, zdd_A_nf), Is().True());
                AssertThat(zdd_subseteq(zdd_B_nf, zdd_B_nf), Is().True());
                AssertThat(zdd_subseteq(zdd_C_nf, zdd_C_nf), Is().True());
                AssertThat(zdd_subseteq(zdd_D_nf, zdd_D_nf), Is().True());
                AssertThat(zdd_subseteq(zdd_E_nf, zdd_E_nf), Is().True());
              });

            it("accepts on equal ZDDs", [&]() {
                AssertThat(zdd_subseteq(zdd_A_nf, zdd_A_nf_copy), Is().True());
                AssertThat(zdd_subseteq(zdd_C_nf, zdd_C_nf_copy), Is().True());
                AssertThat(zdd_subseteq(zdd_D_nf, zdd_D_nf_copy), Is().True());
                AssertThat(zdd_subseteq(zdd_E_nf, zdd_E_nf_copy), Is().True());
              });

            it("can compare Ø and { Ø }", [&]() {
                AssertThat(zdd_subseteq(zdd_empty_nf, zdd_null_nf), Is().True());
                AssertThat(zdd_subseteq(zdd_null_nf, zdd_empty_nf), Is().False());
              });

            it("can compare { {1} } and { {2} }", [&]() {
                AssertThat(zdd_subseteq(zdd_A_nf, zdd_F_nf), Is().False());
                AssertThat(zdd_subseteq(zdd_F_nf, zdd_A_nf), Is().False());
              });

            it("can compare { {1} } and { {1}, {1,2} }", [&]() {
                AssertThat(zdd_subseteq(zdd_A_nf, zdd_B_nf), Is().True());
                AssertThat(zdd_subseteq(zdd_B_nf, zdd_A_nf), Is().False());
              });

            it("can compare { Ø } and { {1}, {1,2} }", [&]() {
                AssertThat(zdd_subseteq(zdd_null_nf, zdd_B_nf), Is().False());
                AssertThat(zdd_subseteq(zdd_B_nf, zdd_null_nf), Is().False());
              });

            it("can compare { Ø, {0,3} } and { {1}, {1,2} }", [&]() {
                AssertThat(zdd_subseteq(zdd_E_nf, zdd_B_nf), Is().False());
                AssertThat(zdd_subseteq(zdd_B_nf, zdd_E_nf), Is().False());
              });

            it("can identify subsets of { Ø, {1}, {2}, {1,2} }", [&]() {
                // Positive cases
                AssertThat(zdd_subseteq(zdd_empty_nf, zdd_C_nf), Is().True());
                AssertThat(zdd_subseteq(zdd_null_nf, zdd_C_nf), Is().True());
                AssertThat(zdd_subseteq(zdd_A_nf, zdd_C_nf), Is().True());
                AssertThat(zdd_subseteq(zdd_B_nf, zdd_C_nf), Is().True());
                AssertThat(zdd_subseteq(zdd_C_nf_copy, zdd_C_nf), Is().True());

                // Negative cases
                AssertThat(zdd_subseteq(zdd_D_nf, zdd_C_nf), Is().False());
                AssertThat(zdd_subseteq(zdd_E_nf, zdd_C_nf), Is().False());
              });

            it("can identify subsets of { {1}, {1,2}, {0,1,3} } and ", [&]() {
                // Positive cases
                AssertThat(zdd_subseteq(zdd_empty_nf, zdd_D_nf), Is().True());
                AssertThat(zdd_subseteq(zdd_A_nf, zdd_D_nf), Is().True());
                AssertThat(zdd_subseteq(zdd_B_nf, zdd_D_nf), Is().True());
                AssertThat(zdd_subseteq(zdd_D_nf_copy, zdd_D_nf), Is().True());

                // Negative cases
                AssertThat(zdd_subseteq(zdd_null_nf, zdd_D_nf), Is().False());
                AssertThat(zdd_subseteq(zdd_C_nf, zdd_D_nf), Is().False());
                AssertThat(zdd_subseteq(zdd_E_nf, zdd_D_nf), Is().False());
              });
          });

        describe("zdd_subset", [&]() {
            it("rejects same file", [&]() {
                AssertThat(zdd_subset(zdd_A_nf, zdd_A_nf), Is().False());
                AssertThat(zdd_subset(zdd_B_nf, zdd_B_nf), Is().False());
                AssertThat(zdd_subset(zdd_C_nf, zdd_C_nf), Is().False());
                AssertThat(zdd_subset(zdd_D_nf, zdd_D_nf), Is().False());
                AssertThat(zdd_subset(zdd_E_nf, zdd_E_nf), Is().False());
              });

            it("rejects on equal ZDDs", [&]() {
                AssertThat(zdd_subset(zdd_A_nf, zdd_A_nf_copy), Is().False());
                AssertThat(zdd_subset(zdd_C_nf, zdd_C_nf_copy), Is().False());
                AssertThat(zdd_subset(zdd_D_nf, zdd_D_nf_copy), Is().False());
                AssertThat(zdd_subset(zdd_E_nf, zdd_E_nf_copy), Is().False());
              });

            it("can compare Ø and { Ø }", [&]() {
                AssertThat(zdd_subset(zdd_empty_nf, zdd_null_nf), Is().True());
                AssertThat(zdd_subset(zdd_null_nf, zdd_empty_nf), Is().False());
              });

            it("can compare { {1} } and { {2} }", [&]() {
                AssertThat(zdd_subset(zdd_A_nf, zdd_F_nf), Is().False());
                AssertThat(zdd_subset(zdd_F_nf, zdd_A_nf), Is().False());
              });

            it("can compare { {1} } and { {1}, {1,2} }", [&]() {
                AssertThat(zdd_subset(zdd_A_nf, zdd_B_nf), Is().True());
                AssertThat(zdd_subset(zdd_B_nf, zdd_A_nf), Is().False());
              });

            it("can compare { Ø } and { {1}, {1,2} }", [&]() {
                AssertThat(zdd_subset(zdd_null_nf, zdd_B_nf), Is().False());
                AssertThat(zdd_subset(zdd_B_nf, zdd_null_nf), Is().False());
              });

            it("can compare { Ø, {0,3} } and { {1}, {1,2} }", [&]() {
                AssertThat(zdd_subset(zdd_E_nf, zdd_B_nf), Is().False());
                AssertThat(zdd_subset(zdd_B_nf, zdd_E_nf), Is().False());
              });

            it("can identify subsets of { Ø, {1}, {2}, {1,2} }", [&]() {
                // Positive cases
                AssertThat(zdd_subset(zdd_empty_nf, zdd_C_nf), Is().True());
                AssertThat(zdd_subset(zdd_null_nf, zdd_C_nf), Is().True());
                AssertThat(zdd_subset(zdd_A_nf, zdd_C_nf), Is().True());
                AssertThat(zdd_subset(zdd_B_nf, zdd_C_nf), Is().True());

                // Equality case
                AssertThat(zdd_subset(zdd_C_nf_copy, zdd_C_nf), Is().False());

                // Negative cases
                AssertThat(zdd_subset(zdd_D_nf, zdd_C_nf), Is().False());
                AssertThat(zdd_subset(zdd_E_nf, zdd_C_nf), Is().False());
              });

            it("can identify subsets of { {1}, {1,2}, {0,1,3} } and ", [&]() {
                // Positive cases
                AssertThat(zdd_subset(zdd_empty_nf, zdd_D_nf), Is().True());
                AssertThat(zdd_subset(zdd_A_nf, zdd_D_nf), Is().True());
                AssertThat(zdd_subset(zdd_B_nf, zdd_D_nf), Is().True());

                // Equality case
                AssertThat(zdd_subset(zdd_D_nf_copy, zdd_D_nf), Is().False());

                // Negative cases
                AssertThat(zdd_subset(zdd_null_nf, zdd_D_nf), Is().False());
                AssertThat(zdd_subset(zdd_C_nf, zdd_D_nf), Is().False());
                AssertThat(zdd_subset(zdd_E_nf, zdd_D_nf), Is().False());
              });
          });

        describe("zdd_disjoint", [&]() {
            it("rejects same file", [&]() {
                AssertThat(zdd_disjoint(zdd_A_nf, zdd_A_nf), Is().False());
                AssertThat(zdd_disjoint(zdd_B_nf, zdd_B_nf), Is().False());
                AssertThat(zdd_disjoint(zdd_C_nf, zdd_C_nf), Is().False());
                AssertThat(zdd_disjoint(zdd_D_nf, zdd_D_nf), Is().False());
                AssertThat(zdd_disjoint(zdd_E_nf, zdd_E_nf), Is().False());
              });

            it("accepts same file for Ø", [&]() {
                AssertThat(zdd_disjoint(zdd_empty_nf, zdd_empty_nf), Is().True());
              });

            it("rejects same file for { Ø }", [&]() {
                AssertThat(zdd_disjoint(zdd_null_nf, zdd_null_nf), Is().False());
              });

            it("rejects on equal ZDDs", [&]() {
                AssertThat(zdd_disjoint(zdd_A_nf, zdd_A_nf_copy), Is().False());
                AssertThat(zdd_disjoint(zdd_C_nf, zdd_C_nf_copy), Is().False());
                AssertThat(zdd_disjoint(zdd_D_nf, zdd_D_nf_copy), Is().False());
                AssertThat(zdd_disjoint(zdd_E_nf, zdd_E_nf_copy), Is().False());
              });

            it("can compare Ø and { Ø }", [&]() {
                AssertThat(zdd_disjoint(zdd_empty_nf, zdd_null_nf), Is().True());
                AssertThat(zdd_disjoint(zdd_null_nf, zdd_empty_nf), Is().True());
              });

            it("accepts Ø and Ø", [&]() {
                AssertThat(zdd_disjoint(zdd_empty_nf, zdd_empty_nf_copy), Is().True());
              });

            it("rejects { Ø } and { Ø }", [&]() {
                AssertThat(zdd_disjoint(zdd_empty_nf, zdd_null_nf), Is().True());
                AssertThat(zdd_disjoint(zdd_null_nf, zdd_empty_nf), Is().True());
              });

            it("accepts Ø and { {1}, {1,2} }", [&]() {
                AssertThat(zdd_disjoint(zdd_empty_nf, zdd_B_nf), Is().True());
                AssertThat(zdd_disjoint(zdd_B_nf, zdd_empty_nf), Is().True());
              });

            it("accepts { {1} } and { {2} }", [&]() {
                AssertThat(zdd_disjoint(zdd_A_nf, zdd_F_nf), Is().True());
                AssertThat(zdd_disjoint(zdd_F_nf, zdd_A_nf), Is().True());
              });

            it("rejects { {1} } and { {1}, {1,2} }", [&]() {
                AssertThat(zdd_disjoint(zdd_A_nf, zdd_B_nf), Is().False());
                AssertThat(zdd_disjoint(zdd_B_nf, zdd_A_nf), Is().False());
              });

            it("accepts { Ø } and { {1}, {1,2} }", [&]() {
                AssertThat(zdd_disjoint(zdd_null_nf, zdd_B_nf), Is().True());
                AssertThat(zdd_disjoint(zdd_B_nf, zdd_null_nf), Is().True());
              });

            it("accepts { Ø, {0,3} } and { {1}, {1,2} }", [&]() {
                AssertThat(zdd_disjoint(zdd_E_nf, zdd_B_nf), Is().True());
                AssertThat(zdd_disjoint(zdd_B_nf, zdd_E_nf), Is().True());
              });

            it("rejects subsets of { Ø, {1}, {2}, {1,2} }", [&]() {
                AssertThat(zdd_disjoint(zdd_D_nf, zdd_C_nf), Is().False());
                AssertThat(zdd_disjoint(zdd_C_nf, zdd_D_nf), Is().False());
              });

            it("rejects subsets of { Ø, {1}, {2}, {1,2} }", [&]() {
                AssertThat(zdd_disjoint(zdd_null_nf, zdd_C_nf), Is().False());
                AssertThat(zdd_disjoint(zdd_A_nf, zdd_C_nf), Is().False());
                AssertThat(zdd_disjoint(zdd_B_nf, zdd_C_nf), Is().False());
                AssertThat(zdd_disjoint(zdd_C_nf_copy, zdd_C_nf), Is().False());
                AssertThat(zdd_disjoint(zdd_E_nf, zdd_C_nf), Is().False());
                AssertThat(zdd_disjoint(zdd_F_nf, zdd_C_nf), Is().False());
              });

            it("accepts non-subsets of { Ø, {1}, {2}, {1,2} }", [&]() {
                AssertThat(zdd_disjoint(zdd_empty_nf, zdd_C_nf), Is().True());
              });

            it("accepts { {2} } and { {1}, {1,2} }", [&]() {
                AssertThat(zdd_disjoint(zdd_F_nf, zdd_B_nf), Is().True());
                AssertThat(zdd_disjoint(zdd_B_nf, zdd_F_nf), Is().True());
              });
          });
      });
  });

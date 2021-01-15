go_bandit([]() {
    describe("BDD: BDD Class", [&]() {
        node_file x0_nf;

        {
          node_writer nw_0(x0_nf);
          nw_0 << create_node(0,MAX_ID,
                              create_sink_ptr(false),
                              create_sink_ptr(true));
        }

        bdd x0(x0_nf);

        node_file x0_and_x1_nf;

        {
          node_writer nw_01(x0_and_x1_nf);

          nw_01 << create_node(1, MAX_ID,
                               create_sink_ptr(false),
                               create_sink_ptr(true));

          nw_01 << create_node(0, MAX_ID,
                               create_sink_ptr(false),
                               create_node_ptr(1, MAX_ID));
        }

        bdd x0_and_x1(x0_and_x1_nf);

        node_file sink_T_nf;

        {
          node_writer nw_T(sink_T_nf);
          nw_T << create_sink(true);
        }

        node_file sink_T(sink_T_nf);

        node_file sink_F_nf;

        {
          node_writer nw_F(sink_F_nf);
          nw_F << create_sink(false);
        }

        bdd sink_F(sink_F_nf);

        describe("is_sink predicate", [&]() {
            it("should reject x0 as a sink file", [&]() {
              AssertThat(is_sink(x0, is_true), Is().False());
              AssertThat(is_sink(x0, is_false), Is().False());
              AssertThat(is_sink(x0), Is().False());
            });

            it("should reject x0 & x1 as a sink file", [&]() {
              AssertThat(is_sink(x0_and_x1, is_true), Is().False());
              AssertThat(is_sink(x0_and_x1, is_false), Is().False());
              AssertThat(is_sink(x0_and_x1), Is().False());
            });

            it("should recognise a true sink", [&]() {
              AssertThat(is_sink(sink_T, is_true), Is().True());
            });

            it("should recognise a false sink", [&]() {
              AssertThat(is_sink(sink_F, is_false), Is().True());
            });

            it("should not recognise sink file as the other sink", [&]() {
              AssertThat(is_sink(sink_T, is_false), Is().False());
              AssertThat(is_sink(sink_F, is_true), Is().False());
            });

            it("should have any sink as default", [&]() {
              AssertThat(is_sink(sink_T), Is().True());
              AssertThat(is_sink(sink_F), Is().True());
            });
        });
    });
});

go_bandit([]() {
    describe("ZDD: ZDD Class", [&]() {
        node_file x0_nf;

        {
          node_writer nw_0(x0_nf);
          nw_0 << create_node(0,MAX_ID,
                              create_sink_ptr(false),
                              create_sink_ptr(true));
        }

        zdd x0(x0_nf);

        node_file x1_nf;

        {
          node_writer nw_1(x1_nf);
          nw_1 << create_node(1,MAX_ID,
                              create_sink_ptr(false),
                              create_sink_ptr(true));
        }

        zdd x1(x1_nf);

        node_file x0_or_x1_nf;

        {
          node_writer nw_01(x0_or_x1_nf);

          nw_01 << create_node(1, MAX_ID,
                               create_sink_ptr(false),
                               create_sink_ptr(true));

          nw_01 << create_node(0, MAX_ID,
                               create_node_ptr(1, MAX_ID),
                               create_sink_ptr(true));
        }

        zdd x0_or_x1(x0_or_x1_nf);

        node_file sink_T_nf;

        {
          node_writer nw_T(sink_T_nf);
          nw_T << create_sink(true);
        }

        zdd sink_T(sink_T_nf);

        node_file sink_F_nf;

        {
          node_writer nw_F(sink_F_nf);
          nw_F << create_sink(false);
        }

        zdd sink_F(sink_F_nf);

        describe("__zdd class", [&]() {
            it("should copy-construct values from zdd", [&]() {
                __zdd t1 = x0_or_x1;
                AssertThat(t1.has<node_file>(), Is().True());
                AssertThat(t1.get<node_file>()._file_ptr, Is().EqualTo(x0_or_x1_nf._file_ptr));
                AssertThat(t1.negate, Is().False());
              });

            it("should copy-construct values from __zdd", [&]() {
                __zdd t1 = x0_or_x1;
                __zdd t2 = t1;
                AssertThat(t2.has<node_file>(), Is().True());
                AssertThat(t2.get<node_file>()._file_ptr, Is().EqualTo(x0_or_x1_nf._file_ptr));
                AssertThat(t2.negate, Is().False());
              });

            it("should copy-construct values from node_file", [&]() {
                __zdd t1 = x0_or_x1;
                AssertThat(t1.has<node_file>(), Is().True());
                AssertThat(t1.get<node_file>()._file_ptr, Is().EqualTo(x0_or_x1_nf._file_ptr));
                AssertThat(t1.negate, Is().False());
              });

            arc_file af;

            {
              arc_writer aw(af);
              aw.unsafe_push_node(arc {create_node_ptr(0,0), create_node_ptr(1,0)});

              aw.unsafe_push_sink(arc {flag(create_node_ptr(0,0)), create_sink_ptr(true)});
              aw.unsafe_push_sink(arc {create_node_ptr(1,0), create_sink_ptr(false)});
              aw.unsafe_push_sink(arc {flag(create_node_ptr(1,0)), create_sink_ptr(true)});

              aw.unsafe_push(create_level_info(0,1u));
              aw.unsafe_push(create_level_info(1,1u));
            }

            it("should copy-construct values from arc_file", [&]() {
                __zdd t1 = af;
                AssertThat(t1.has<arc_file>(), Is().True());
                AssertThat(t1.get<arc_file>()._file_ptr, Is().EqualTo(af._file_ptr));
                AssertThat(t1.negate, Is().False());
              });

            it("should reduce on copy construct to zdd with arc_file", [&]() {
                zdd out = __zdd(af);
                AssertThat(out, Is().EqualTo(x0_or_x1));
              });
          });

        it("should copy-construct node_file and negation back to zdd", [&]() {
            // Since we know the __zdd copy constructor works, then we can use
            // it to peek into the 'zdd' class
            __zdd t2 = __zdd(zdd(__zdd(x0_or_x1)));
            AssertThat(t2.has<node_file>(), Is().True());
            AssertThat(t2.get<node_file>()._file_ptr, Is().EqualTo(x0_or_x1_nf._file_ptr));
            AssertThat(t2.negate, Is().False());
         });


        describe("operators", [&]() {
            it("should reject Ø == {Ø}", [&]() {
                AssertThat(sink_F, Is().Not().EqualTo(sink_T));
              });

            it("should accept {{x0}} == {{x0}} (different files)", [&]() {
                node_file x0_nf_2;

                { node_writer nw_0(x0_nf_2);
                  nw_0 << create_node(0,MAX_ID,
                                      create_sink_ptr(false),
                                      create_sink_ptr(true));
                }

                zdd x0_2(x0_nf);

                AssertThat(x0, Is().EqualTo(x0_2));
              });

            it("should compute {{x0}} /\\ {{x1}} == {{x0}, {x1}}", [&]() {
                AssertThat((x0 | x1) == x0_or_x1, Is().True());
              });

            it("should compute {{x0}} \\/ {{x0},{x1}} == {{x0}}", [&]() {
                AssertThat((x0 & x0_or_x1) == x0, Is().True());
              });

            it("should compute {{x0},{x1}} \\ {{x0} == {{x0}}", [&]() {
                AssertThat((x0_or_x1 - x0) == x1, Is().True());
              });

            it("should compute with __zdd&& operators", [&]() {
                zdd out = ((x0_or_x1 - x0) | ((x0 | x1) & (x0_or_x1 - x1))) - (x0_or_x1 - x0);
                AssertThat(x0, Is().EqualTo(out));
              });

            it("should ?= __zdd&&", [&]() {
                zdd out = x0_or_x1;
                out -= x1;
                AssertThat(out, Is().EqualTo(x0));

                out |= x0_or_x1 & x1;
                AssertThat(out, Is().EqualTo(x0_or_x1));

                out &= x0_or_x1 & x1;
                AssertThat(out, Is().EqualTo(x1));
             });

            it("should check two derivations of same __bdd&&", [&]() {
                AssertThat((x0 | x1) == ((x0_or_x1 - x1) | x1), Is().True());
                AssertThat((x0 | x1) != ((x0_or_x1 - x1) | x1), Is().False());
             });

            it("should check two derivations of different __bdd&&", [&]() {
                AssertThat((x0 | x1) == (x0_or_x1 - x1), Is().False());
                AssertThat((x0 | x1) != (x0_or_x1 - x1), Is().True());
             });
          });

        it("should copy-construct node_file and negation back to zdd", [&]() {
            // Since we know the __zdd copy constructor works, then we can use
            // it to peek into the 'zdd' class
            __zdd t2 = __zdd(zdd(__zdd(x0_or_x1)));
            AssertThat(t2.has<node_file>(), Is().True());
            AssertThat(t2.get<node_file>()._file_ptr, Is().EqualTo(x0_or_x1_nf._file_ptr));
            AssertThat(t2.negate, Is().False());
          });
      });
  });

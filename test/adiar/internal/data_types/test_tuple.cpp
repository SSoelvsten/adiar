go_bandit([]() {
    describe("adiar/internal/data_types/tuple.h", []() {
        describe("tuple<int, 1>", []() {
            // TODO
          });

        describe("tuple<int, 2>", []() {
            it("statically provides its cardinality of 2.", []() {
                const auto cardinality = tuple<int, 2>::CARDINALITY;
                AssertThat(cardinality, Is().EqualTo(2u));
              });

            describe("unordered access: '[]' operator", []() {
                it("provides unordered access with '[]' operator [42,22]", []() {
                    const tuple<int,2> t = { 42,22 };

                    AssertThat(t[0], Is().EqualTo(42));
                    AssertThat(t[1], Is().EqualTo(22));
                  });

                it("provides unordered access with '[]' operator [0, -1]", []() {
                    const tuple<int,2> t = { 0, -1 };

                    AssertThat(t[0], Is().EqualTo(0));
                    AssertThat(t[1], Is().EqualTo(-1));
                  });

                // TODO: Index out-of-bounds?
              });

            describe("unordered access: '[]' operator", []() {
                it("provides ordered access to first element", []() {
                    const tuple<int,2> t1 = { 21, 42 };
                    AssertThat(t1.fst(), Is().EqualTo(21));

                    const tuple<int,2> t2 = { 42, 21 };
                    AssertThat(t2.fst(), Is().EqualTo(21));
                  });

                it("provides ordered access to first element [negative value]", []() {
                    const tuple<int,2> t = { -2, -1 };
                    AssertThat(t.fst(), Is().EqualTo(-2));
                  });

                it("provides ordered access to first element [0 vs. 1]", []() {
                    const tuple<int,2> t1 = { 1, 0 };
                    AssertThat(t1.fst(), Is().EqualTo(0));

                    const tuple<int,2> t2 = { 0, 1 };
                    AssertThat(t2.fst(), Is().EqualTo(0));
                  });

                it("provides ordered access to first element [tie]", []() {
                    const tuple<int,2> t = { 21, 21 };
                    AssertThat(t.fst(), Is().EqualTo(21));
                  });

                it("provides ordered access to second element", []() {
                    const tuple<int,2> t1 = { 21, 42 };
                    AssertThat(t1.snd(), Is().EqualTo(42));

                    const tuple<int,2> t2 = { 42, 21 };
                    AssertThat(t2.snd(), Is().EqualTo(42));
                  });

                it("provides ordered access to second element [negative values]", []() {
                    const tuple<int,2> t = { -2, -1 };
                    AssertThat(t.snd(), Is().EqualTo(-1));
                  });

                it("provides ordered access to second element [0 vs. 1]", []() {
                    const tuple<int,2> t1 = { 1, 0 };
                    AssertThat(t1.snd(), Is().EqualTo(1));

                    const tuple<int,2> t2 = { 0, 1 };
                    AssertThat(t2.snd(), Is().EqualTo(1));
                  });

                it("provides ordered access to second element [tie]", []() {
                    const tuple<int,2> t = { 21, 21 };
                    AssertThat(t.snd(), Is().EqualTo(21));
                  });
              });

            it("can construct a tuple with copies of a single element [-1]", []() {
                const tuple<int,2> t = { -1 };

                AssertThat(t[0], Is().EqualTo(-1));
                AssertThat(t[1], Is().EqualTo(-1));
              });

            it("can construct a tuple with copies of a single element [0]", []() {
                const tuple<int,2> t = { 0 };

                AssertThat(t[0], Is().EqualTo(0));
                AssertThat(t[1], Is().EqualTo(0));
              });

            it("can construct a tuple with copies of a single element [1]", []() {
                const tuple<int,2> t = { 1 };

                AssertThat(t[0], Is().EqualTo(1));
                AssertThat(t[1], Is().EqualTo(1));
              });

            it("can construct a tuple with copies of a single element [42]", []() {
                const tuple<int,2> t = { 42 };

                AssertThat(t[0], Is().EqualTo(42));
                AssertThat(t[1], Is().EqualTo(42));
              });

            describe("Lexicographical Ordering '<'", []() {
                it("{0, 2} < {1, 2}", []() {
                    const tuple<int,2> t1 = { 0, 2 };
                    const tuple<int,2> t2 = { 1, 2 };

                    AssertThat(t1, Is().LessThan(t2));
                    AssertThat(t2, Is().Not().LessThan(t1));
                    AssertThat(t1, Is().Not().GreaterThan(t2));
                  });

                it("{1, 2} < {2, 0}", []() {
                    const tuple<int,2> t1 = { 0, 2 };
                    const tuple<int,2> t2 = { 1, 0 };

                    AssertThat(t1, Is().LessThan(t2));
                    AssertThat(t2, Is().Not().LessThan(t1));
                    AssertThat(t1, Is().Not().GreaterThan(t2));
                  });

                it("{0, 0} < {0, 1}", []() {
                    const tuple<int,2> t1 = { 0, 0 };
                    const tuple<int,2> t2 = { 0, 1 };

                    AssertThat(t1, Is().LessThan(t2));
                    AssertThat(t2, Is().Not().LessThan(t1));
                    AssertThat(t1, Is().Not().GreaterThan(t2));
                  });

                it("{0, 0} < {0, 1}", []() {
                    const tuple<int,2> t1 = { 0, 0 };
                    const tuple<int,2> t2 = { 0, 1 };

                    AssertThat(t1, Is().LessThan(t2));
                    AssertThat(t2, Is().Not().LessThan(t1));
                    AssertThat(t1, Is().Not().GreaterThan(t2));
                  });
              });

            describe("tuple_fst_lt", []() {
                it("{0, 2} < {1, 2}", []() {
                    const tuple<int,2> t1 = { 0, 2 };
                    const tuple<int,2> t2 = { 1, 2 };

                    const bool lt_lr = tuple_fst_lt<tuple<int,2>>()(t1,t2);
                    AssertThat(lt_lr, Is().True());

                    const bool lt_rl = tuple_fst_lt<tuple<int,2>>()(t2,t1);
                    AssertThat(lt_rl, Is().False());
                  });

                it("{2, 0} < {1, 2}", []() {
                    const tuple<int,2> t1 = { 2, 0 };
                    const tuple<int,2> t2 = { 1, 2 };

                    const bool lt_lr = tuple_fst_lt<tuple<int,2>>()(t1,t2);
                    AssertThat(lt_lr, Is().True());

                    const bool lt_rl = tuple_fst_lt<tuple<int,2>>()(t2,t1);
                    AssertThat(lt_rl, Is().False());
                  });

                it("{0, 1} < {2, 0}", []() {
                    const tuple<int,2> t1 = { 0, 1 };
                    const tuple<int,2> t2 = { 2, 0 };

                    const bool lt_lr = tuple_fst_lt<tuple<int,2>>()(t1,t2);
                    AssertThat(lt_lr, Is().True());

                    const bool lt_rl = tuple_fst_lt<tuple<int,2>>()(t2,t1);
                    AssertThat(lt_rl, Is().False());
                  });
              });

            describe("tuple_snd_lt", []() {
                it("{1, 0} < {2, 0}", []() {
                    const tuple<int,2> t1 = { 1, 0 };
                    const tuple<int,2> t2 = { 2, 0 };

                    const bool lt_lr = tuple_snd_lt<tuple<int,2>>()(t1,t2);
                    AssertThat(lt_lr, Is().True());

                    const bool lt_rl = tuple_snd_lt<tuple<int,2>>()(t2,t1);
                    AssertThat(lt_rl, Is().False());
                  });

                it("{2, 1} < {3, 0}", []() {
                    const tuple<int,2> t1 = { 2, 1 };
                    const tuple<int,2> t2 = { 3, 0 };

                    const bool lt_lr = tuple_snd_lt<tuple<int,2>>()(t1,t2);
                    AssertThat(lt_lr, Is().True());

                    const bool lt_rl = tuple_snd_lt<tuple<int,2>>()(t2,t1);
                    AssertThat(lt_rl, Is().False());
                  });

                it("{2, 3} < {3, 1}", []() {
                    const tuple<int,2> t1 = { 2, 3 };
                    const tuple<int,2> t2 = { 3, 1 };

                    const bool lt_lr = tuple_snd_lt<tuple<int,2>>()(t1,t2);
                    AssertThat(lt_lr, Is().True());

                    const bool lt_rl = tuple_snd_lt<tuple<int,2>>()(t2,t1);
                    AssertThat(lt_rl, Is().False());
                  });

                it("{2, -1} < {2, 0}", []() {
                    const tuple<int,2> t1 = { 2, -1 };
                    const tuple<int,2> t2 = { 2, 0 };

                    const bool lt_lr = tuple_snd_lt<tuple<int,2>>()(t1,t2);
                    AssertThat(lt_lr, Is().True());

                    const bool lt_rl = tuple_snd_lt<tuple<int,2>>()(t2,t1);
                    AssertThat(lt_rl, Is().False());
                  });
              });
          });

        describe("tuple<int, 2, sorted>", []() {
            // TODO
          });

        describe("tuple<int, 3>", []() {
            // TODO
          });
      });
  });

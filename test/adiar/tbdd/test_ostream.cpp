#include "../../test.h"

#include <adiar/tbdd/ostream.h>

#include <sstream>

struct test__rat_pivot
{
  const int pivot_clause;
  const std::vector<int> hints;
};

go_bandit([]() {
  describe("adiar/tbdd/ostream.h", []() {
    describe("tbdd_ostream<Integer>(..., std::ostream&, ...)", []() {
      describe("type = DRAT, mode = ASCII", []() {
        tbdd_ostream<>::type t = tbdd_ostream<>::DRAT;
        tbdd_ostream<>::mode m = tbdd_ostream<>::ASCII;

        const auto none = tbdd_ostream<>::none;

        // Constructor
        it("throws if 'cnf_vars' is negative", [&]() {
          std::ostringstream out;
          AssertThrows(invalid_argument, tbdd_ostream<>(-1, 3, out, t, m));
        });

        it("throws if 'cnf_clauses' is negative", [&]() {
          std::ostringstream out;
          AssertThrows(invalid_argument, tbdd_ostream<>(42, -3, out, t, m));
        });

        // Extension Variable Counter
        describe(".next_var()", [&]() {
          it("provides fresh extension variables succeeding 'cnf_vars' [0]", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(0, 0, out, t, m);

            AssertThat(to.next_var(), Is().EqualTo(1));
          });

          it("provides fresh extension variables succeeding 'cnf_vars' [1]", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(1, 2, out, t, m);

            AssertThat(to.next_var(), Is().EqualTo(2));
          });

          it("provides fresh extension variables succeeding 'cnf_vars' [2]", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(2, 3, out, t, m);

            AssertThat(to.next_var(), Is().EqualTo(3));
          });

          it("provides fresh extension variables in ascending order", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(1, 1, out, t, m);

            AssertThat(to.next_var(), Is().EqualTo(2));
            AssertThat(to.next_var(), Is().EqualTo(3));
          });
        });

        // RUP Statements
        describe(".write_rup(...), .skip_rup()", [&]() {
          it("formats RUP '1 -2'", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(4, 4, out, t, m);

            const std::vector<int> clause = { 1, -2 };
            to.write_rup(clause.begin(), clause.end());

            AssertThat(out.str(), Is().EqualTo("1 -2 0\n"));
          });

          it("formats RUP '-5'", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(5, 2, out, t, m);

            const std::vector<int> clause = { -5 };
            to.write_rup(clause.begin(), clause.end());

            AssertThat(out.str(), Is().EqualTo("-5 0\n"));
          });

          it("formats RUP ''", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(3, 4, out, t, m);

            const std::vector<int> clause = { };
            to.write_rup(clause.begin(), clause.end());

            AssertThat(out.str(), Is().EqualTo("0\n"));
          });

          it("does not sort RUP '1 3 -2'", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(3, 4, out, t, m);

            const std::vector<int> clause = { 1, 3, -2 };
            to.write_rup(clause.begin(), clause.end());

            AssertThat(out.str(), Is().EqualTo("1 3 -2 0\n"));
          });

          it("ignores hints for RUP '-2 3 1' with '1 2'", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(3, 2, out, t, m);

            const std::vector<int> clause = { -2, 3, 1 };
            const std::vector<int> hints  = { 1, 2 };

            to.write_rup(clause.begin(), clause.end(),
                   hints.begin(), hints.end());

            AssertThat(out.str(), Is().EqualTo("-2 3 1 0\n"));
          });

          it("returns clause_id 'none' for RUP '1 -3 -2 -5'", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(5, 8, out, t, m);

            const std::vector<int> clause = { 1, -3, -2, -5 };

            const auto next_id = to.write_rup(clause.begin(), clause.end());

            AssertThat(next_id, Is().EqualTo(none));
            AssertThat(out.str(), Is().EqualTo("1 -3 -2 -5 0\n"));
          });

          it("skipping RUP does not change output buffer", [&]() {
            std::ostringstream out;
            out << "before";

            tbdd_ostream<> to(42, 129, out, t, m);
            to.skip_rup();

            AssertThat(out.str(), Is().EqualTo("before"));
          });

          it("skipping RUP provides 'none' clause id", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(42, 129, out, t, m);

            AssertThat(to.skip_rup(), Is().EqualTo(none));
          });
        });

        describe(".write_rat(...), skip_rat()", [&]() {
          it("formats RAT 'e -> 1 2 5' with e = next_var()", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(5, 8, out, t, m);

            const auto extension_var = to.next_var();
            const std::vector<int> clause = { -extension_var, 1, 2, 5 };

            const auto next_id = to.write_rat(clause.begin(), clause.end());

            AssertThat(next_id, Is().EqualTo(none));
            AssertThat(out.str(), Is().EqualTo("-6 1 2 5 0\n"));
          });

          it("ignores hints for RAT 'e -> 4 2' with e = next_var()", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(5, 8, out, t, m);

            const auto extension_var = to.next_var();
            const std::vector<int> clause = { -extension_var, 4, 2 };
            const std::vector<test__rat_pivot> pivots = {
              test__rat_pivot{ 7, {{ none, none }} }
            };

            const auto next_id = to.write_rat(clause.begin(), clause.end());

            AssertThat(next_id, Is().EqualTo(none));
            AssertThat(out.str(), Is().EqualTo("-6 4 2 0\n"));
          });

          it("skipping RUP does not change output buffer", [&]() {
            std::ostringstream out;
            out << "before";

            tbdd_ostream<> to(42, 129, out, t, m);
            to.skip_rat();

            AssertThat(out.str(), Is().EqualTo("before"));
          });

          it("skipping RUP provides 'none' clause id", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(42, 129, out, t, m);

            AssertThat(to.skip_rat(), Is().EqualTo(none));
          });
        });

        describe(".write_del(...)", [&]() {
          it("formats clause deletion of '1 3 5'", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(5, 8, out, t, m);

            const std::vector<int> clause = { 1, 3, 5 };

            to.write_del(clause.begin(), clause.end());

            AssertThat(out.str(), Is().EqualTo("d 1 3 5 0\n"));
          });

          it("formats clause deletion of '-2 3'", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(5, 8, out, t, m);

            const std::vector<int> clause = { -2, 3 };

            to.write_del(clause.begin(), clause.end());

            AssertThat(out.str(), Is().EqualTo("d -2 3 0\n"));
          });
        });

        describe(".size(), .file_size()", [&]() {
          it("updates size for RUP statements", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(5, 8, out, t, m);

            const std::vector<int> clause = { -2, 3 };
            to.write_rup(clause.begin(), clause.end());

            AssertThat(to.size(), Is().EqualTo(clause.size()));
            AssertThat(to.file_size(), Is().EqualTo(std::string("-2 3 0\n").size()));
          });

          it("updates size for RAT statements", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(5, 8, out, t, m);

            const std::vector<int> clause = { 5, -4, 2 };
            to.write_rat(clause.begin(), clause.end());

            AssertThat(to.size(), Is().EqualTo(clause.size()));
            AssertThat(to.file_size(), Is().EqualTo(std::string("5 -4 2 0\n").size()));
          });

          it("updates size for DEL statements", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(5, 8, out, t, m);

            const std::vector<int> clause = { -4, 2 };
            to.write_del(clause.begin(), clause.end());

            AssertThat(to.size(), Is().EqualTo(clause.size()));
            AssertThat(to.file_size(), Is().EqualTo(std::string("d -4 2 0\n").size()));
          });

          it("continually provides the size for empty std::stringstream", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(5, 8, out, t, m);

            AssertThat(to.file_size(), Is().EqualTo(0u));

            {
              const std::vector<int> clause = { -2, 3 };
              to.write_rup(clause.begin(), clause.end());

              AssertThat(to.size(), Is().EqualTo(2u));
              AssertThat(to.file_size(), Is().EqualTo(std::string("-2 3 0\n").size()));
            }
            {
              const std::vector<int> clause = { 3 };
              to.write_rat(clause.begin(), clause.end());

              AssertThat(to.size(), Is().EqualTo(3u));
              AssertThat(to.file_size(), Is().EqualTo(std::string("-2 3 0\n3 0\n").size()));
            }
            {
              const std::vector<int> clause = { -2 };
              to.write_del(clause.begin(), clause.end());

              AssertThat(to.size(), Is().EqualTo(4u));
              AssertThat(to.file_size(), Is().EqualTo(std::string("-2 3 0\n3 0\nd -2 0\n").size()));
            }
          });

          it("accounts for being given non-empty stream", [&]() {
            std::ostringstream out;
            out << "some text";

            tbdd_ostream<> to(5, 8, out, t, m);

            AssertThat(to.size(), Is().EqualTo(0u));
            AssertThat(to.file_size(), Is().EqualTo(0u));
          });
        });

        it("recreates DRUP example in Fig. 1 [Cruz-Filipe 2017]", [&]() {
          std::ostringstream out;
          tbdd_ostream<> to(4, 8, out, t, m);

          {
            const std::vector<int> clause = { 1, 2 };
            to.write_rup(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 1, -3, 2 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 1, 3 };
            to.write_rup(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 1, 4, 3 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 1 };
            to.write_rup(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 1, 3 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 1, 2 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 1, -4, -2 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 2 };
            to.write_rup(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { -1, 4, 2 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 2, -4, 3 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { };
            to.write_rup(clause.begin(), clause.end());
          }

          std::string expected = R"(1 2 0
d 1 -3 2 0
1 3 0
d 1 4 3 0
1 0
d 1 3 0
d 1 2 0
d 1 -4 -2 0
2 0
d -1 4 2 0
d 2 -4 3 0
0
)";

          AssertThat(out.str(), Is().EqualTo(expected));

          AssertThat(to.size(), Is().EqualTo(25u));
          AssertThat(to.file_size(), Is().EqualTo(expected.size()));
        });

        it("recreates DRAT example in Fig. 2 [Cruz-Filipe 2017]", [&]() {
          std::ostringstream out;
          tbdd_ostream<> to(4, 8, out, t, m);

          {
            const std::vector<int> clause = { 1 };
            to.write_rat(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 1, -4, -2 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 1, 4, 3 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 1, 2, -3 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 2 };
            to.write_rup(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { -1, 2, 4 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 2, -4, 3 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { };
            to.write_rup(clause.begin(), clause.end());
          }

          std::string expected = R"(1 0
d 1 -4 -2 0
d 1 4 3 0
d 1 2 -3 0
2 0
d -1 2 4 0
d 2 -4 3 0
0
)";

          AssertThat(out.str(), Is().EqualTo(expected));

          AssertThat(to.size(), Is().EqualTo(17u));
          AssertThat(to.file_size(), Is().EqualTo(expected.size()));
        });
      });

      describe("type = LRAT, mode = ASCII", []() {
        tbdd_ostream<>::type t = tbdd_ostream<>::LRAT;
        tbdd_ostream<>::mode m = tbdd_ostream<>::ASCII;

        // Constructor
        it("throws if 'cnf_vars' is negative", [&]() {
          std::ostringstream out;
          AssertThrows(invalid_argument, tbdd_ostream<>(-1, 3, out, t, m));
        });

        it("throws if 'cnf_clauses' is negative", [&]() {
          std::ostringstream out;
          AssertThrows(invalid_argument, tbdd_ostream<>(42, -3, out, t, m));
        });

        // Extension Variable Counter
        describe(".next_var()", [&]() {
          it("provides fresh extension variables in ascending order", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(42, 1273, out, t, m);

            AssertThat(to.next_var(), Is().EqualTo(43));
            AssertThat(to.next_var(), Is().EqualTo(44));
            AssertThat(to.next_var(), Is().EqualTo(45));
          });
        });

        describe(".write_rup(...), .skip_rup()", [&]() {
          it("formats RUP '1 -2' with hints '2 1 3' and returns its id", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(2, 4, out, t, m);

            const std::vector<int> clause = { 1, -2 };
            const std::vector<int> hints  = { 2, 1, 3 };

            const auto res = to.write_rup(clause.begin(), clause.end(),
                                          hints.begin(), hints.end());

            AssertThat(out.str(), Is().EqualTo("5 1 -2 0 2 1 3 0\n"));
            AssertThat(res, Is().EqualTo(5));
          });

          it("formats RUP '3 -7 4' with hints '32 12 9' and returns its id", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(7, 42, out, t, m);

            const std::vector<int> clause = { 3, -7, 4 };
            const std::vector<int> hints  = { 32, 12, 9 };

            const auto res = to.write_rup(clause.begin(), clause.end(),
                                          hints.begin(), hints.end());

            AssertThat(out.str(), Is().EqualTo("43 3 -7 4 0 32 12 9 0\n"));
            AssertThat(res, Is().EqualTo(43));
          });

          it("can skip initial RUP clause id", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(42, 129, out, t, m);

            AssertThat(to.skip_rup(), Is().EqualTo(130));

            const std::vector<int> clause = { 12, 13 };
            const std::vector<int> hints  = { 69, 8, 4, 42, 56, 112 };

            const auto res = to.write_rup(clause.begin(), clause.end(),
                                          hints.begin(), hints.end());

            AssertThat(out.str(), Is().EqualTo("131 12 13 0 69 8 4 42 56 112 0\n"));
            AssertThat(res, Is().EqualTo(131));
          });

          it("can skip intermediate clause ids", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(43, 5, out, t, m);

            {
              const std::vector<int> clause = { 12, -42 };
              const std::vector<int> hints  = { 2, 5, 4, 3 };

              const auto res = to.write_rup(clause.begin(), clause.end(),
                                            hints.begin(), hints.end());

              AssertThat(out.str(), Is().EqualTo("6 12 -42 0 2 5 4 3 0\n"));
              AssertThat(res, Is().EqualTo(6));
            }

            AssertThat(to.skip_rup(), Is().EqualTo(7));
            AssertThat(to.skip_rup(), Is().EqualTo(8));

            {
              const std::vector<int> clause = { 21, -12 };
              const std::vector<int> hints  = { 6, 1, 4 };

              const auto res = to.write_rup(clause.begin(), clause.end(),
                                            hints.begin(), hints.end());

              AssertThat(out.str(), Is().EqualTo("6 12 -42 0 2 5 4 3 0\n9 21 -12 0 6 1 4 0\n"));
              AssertThat(res, Is().EqualTo(9));
            }
          });
        });

        describe(".write_rat(...), .skip_rat()", [&]() {
          it("formats RAT '-2 -1' with hints '-4 1 3' and returns its id", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(2, 4, out, t, m);

            const std::vector<int> clause = { -2, -1 };
            const std::vector<test__rat_pivot> pivots  = {
              test__rat_pivot{ 4, {{ 1, 3 }} }
            };

            const auto res = to.write_rat(clause.begin(), clause.end(),
                                          pivots.begin(), pivots.end());

            AssertThat(out.str(), Is().EqualTo("5 -2 -1 0 -4 1 3 0\n"));
            AssertThat(res, Is().EqualTo(5));
          });

          it("formats RAT '7 -4 9' with hints '-21 28 4 -8 7 3 4' and returns its id", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(9, 41, out, t, m);

            const std::vector<int> clause = { 7, -4, 9 };
            const std::vector<test__rat_pivot> pivots  = {
              test__rat_pivot{ 21, {{ 28, 4 }} },
              test__rat_pivot{ 8,  {{ 7, 3, 4 }} }
            };

            const auto res = to.write_rat(clause.begin(), clause.end(),
                                          pivots.begin(), pivots.end());

            AssertThat(out.str(), Is().EqualTo("42 7 -4 9 0 -21 28 4 -8 7 3 4 0\n"));
            AssertThat(res, Is().EqualTo(42));
          });

          it("formats RAT '3 -4' with hints '-10' and returns its id", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(4, 12, out, t, m);

            const std::vector<int> clause = { 3, -4 };
            const std::vector<test__rat_pivot> pivots  = {
              test__rat_pivot{ 10, std::vector<int>() }
            };
            AssertThat(pivots.at(0).hints.begin(), Is().EqualTo(pivots.at(0).hints.end()));

            const auto res = to.write_rat(clause.begin(), clause.end(),
                                          pivots.begin(), pivots.end());

            AssertThat(out.str(), Is().EqualTo("13 3 -4 0 -10 0\n"));
            AssertThat(res, Is().EqualTo(13));
          });

          it("can skip initial RAT clause id", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(48, 41, out, t, m);

            AssertThat(to.skip_rat(), Is().EqualTo(42));

            const std::vector<int> clause = { 42, 21 };
            const std::vector<test__rat_pivot> pivots  = {
              test__rat_pivot{ 8, {{ 12 }} }
            };

            const auto res = to.write_rat(clause.begin(), clause.end(),
                                        pivots.begin(), pivots.end());

            AssertThat(out.str(), Is().EqualTo("43 42 21 0 -8 12 0\n"));
            AssertThat(res, Is().EqualTo(43));
          });

          it("can skip intermediate clause ids", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(43, 40, out, t, m);

            {
              const std::vector<int> clause = { 8, 12, 42, -7 };
              const std::vector<test__rat_pivot> pivots  = {
                test__rat_pivot{ 1, {{ 10, 13, 32 }} },
                test__rat_pivot{ 2, {{ 10, 12, 39 }} },
                test__rat_pivot{ 9, {{ 32, 40, 18 }} }
              };

              const auto res = to.write_rat(clause.begin(), clause.end(),
                                            pivots.begin(), pivots.end());

              AssertThat(out.str(), Is().EqualTo("41 8 12 42 -7 0 -1 10 13 32 -2 10 12 39 -9 32 40 18 0\n"));
              AssertThat(res, Is().EqualTo(41));
            }

            AssertThat(to.skip_rat(), Is().EqualTo(42));
            AssertThat(to.skip_rat(), Is().EqualTo(43));

            {
              const std::vector<int> clause = { -42, 8 };
              const std::vector<test__rat_pivot> pivots  = {
                test__rat_pivot{ 4, {{ 3, 16 }} }
              };

              const auto res = to.write_rat(clause.begin(), clause.end(),
                                          pivots.begin(), pivots.end());

              AssertThat(out.str(), Is().EqualTo("41 8 12 42 -7 0 -1 10 13 32 -2 10 12 39 -9 32 40 18 0\n44 -42 8 0 -4 3 16 0\n"));
              AssertThat(res, Is().EqualTo(44));
            }
          });
        });

        describe(".write_del(...)", [&]() {
          it("formats clause deletion of '1 3 8' at beginning", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(5, 8, out, t, m);

            const std::vector<int> clauses = { 1, 3, 8 };

            to.write_del(clauses.begin(), clauses.end());

            AssertThat(out.str(), Is().EqualTo("8 d 1 3 8 0\n"));
          });

          it("formats clause deletion of '1 3 8' after RUP", [&]() {
            std::ostringstream out;
            tbdd_ostream<> to(5, 8, out, t, m);

            {
              const std::vector<int> clause = { 1, -2 };
              const std::vector<int> hints  = { 8, 4 };

              to.write_rup(clause.begin(), clause.end(), hints.begin(), hints.end());
            }
            {
              const std::vector<int> clauses = { 4, 8 };
              to.write_del(clauses.begin(), clauses.end());
            }
            AssertThat(out.str(), Is().EqualTo("9 1 -2 0 8 4 0\n9 d 4 8 0\n"));
          });
        });

        it("recreates LRAT example in Fig. 1 [Cruz-Filipe 2017]", [&]() {
          std::ostringstream out;
          tbdd_ostream<> to(4, 8, out, t, m);

          {
            const std::vector<int> clause = { 1, 2 };
            const std::vector<int> hints  = { 1, 6, 3 };
            to.write_rup(clause.begin(), clause.end(), hints.begin(), hints.end());
          }
          {
            const std::vector<int> clause = { 1 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 1, 3 };
            const std::vector<int> hints  = { 9, 8, 6 };
            to.write_rup(clause.begin(), clause.end(), hints.begin(), hints.end());
          }
          {
            const std::vector<int> clause = { 6 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 1 };
            const std::vector<int> hints  = { 10, 9, 4, 8 };
            to.write_rup(clause.begin(), clause.end(), hints.begin(), hints.end());
          }
          {
            const std::vector<int> clause = { 10, 9, 8 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { 2 };
            const std::vector<int> hints  = { 11, 7, 5, 3 };
            to.write_rup(clause.begin(), clause.end(), hints.begin(), hints.end());
          }
          {
            const std::vector<int> clause = { 7, 3 };
            to.write_del(clause.begin(), clause.end());
          }
          {
            const std::vector<int> clause = { };
            const std::vector<int> hints  = { 11, 12, 2, 4, 5 };
            to.write_rup(clause.begin(), clause.end(), hints.begin(), hints.end());
          }

          std::string expected = R"(9 1 2 0 1 6 3 0
9 d 1 0
10 1 3 0 9 8 6 0
10 d 6 0
11 1 0 10 9 4 8 0
11 d 10 9 8 0
12 2 0 11 7 5 3 0
12 d 7 3 0
13 0 11 12 2 4 5 0
)";

          AssertThat(out.str(), Is().EqualTo(expected));

          AssertThat(to.size(), Is().EqualTo(32u));
          AssertThat(to.file_size(), Is().EqualTo(expected.size()));
        });

        it("recreates LRAT example in Fig. 2 [Cruz-Filipe 2017]", [&]() {
          // TODO
        });

        // TODO: check is-default
      });

      describe("type = LRAT, mode = Binary_Fixed", []() {
        // TODO
      });

      describe("type = LRAT, mode = Binary_Variadic", []() {
        // TODO
      });
    });

    describe("tbdd_ostream<Integer>(..., const std::string&, ...)", []() {
      // TODO
    });
  });
 });

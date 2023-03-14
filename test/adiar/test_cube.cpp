#include "../test.h"

go_bandit([]() {
  describe("adiar/cube.h", []() {
    it("is empty with default constructor", []() {
      const cube c;
      AssertThat(c.vars(),  Is().EqualTo(0u));
      AssertThat(c.empty(), Is().True());
      AssertThat(c.dimensionality(), Is().EqualTo(0u));
    });

    it("wraps a shared_file<cube::pair_t>", []() {
      adiar::shared_file<cube::pair_t> c_file;
      { adiar::file_writer<cube::pair_t> cw(c_file);
        cw << cube::pair_t(0, cube::value_t::False)
           << cube::pair_t(1, cube::value_t::True)
           << cube::pair_t(2, cube::value_t::Dont_Care);
      }

      const cube c(c_file);

      cube_stream cs(c);

      AssertThat(cs.can_pull(), Is().True());
      AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(0, cube::value_t::False)));

      AssertThat(cs.can_pull(), Is().True());
      AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(1, cube::value_t::True)));

      AssertThat(cs.can_pull(), Is().True());
      AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(2, cube::value_t::Dont_Care)));

      AssertThat(cs.can_pull(), Is().False());
    });

    it("reports dimensionality of { 0->0, 1->1, 2->1 } as 0", []() {
      adiar::shared_file<cube::pair_t> c_file;
      { adiar::file_writer<cube::pair_t> cw(c_file);
        cw << cube::pair_t(0, cube::value_t::False)
           << cube::pair_t(1, cube::value_t::True)
           << cube::pair_t(2, cube::value_t::True)
          ;
      }

      const cube c(c_file);
      AssertThat(c.dimensionality(), Is().EqualTo(0u));
    });

    it("reports dimensionality of { 0->0, 1->1, 2->2 } as 1", []() {
      adiar::shared_file<cube::pair_t> c_file;
      { adiar::file_writer<cube::pair_t> cw(c_file);
        cw << cube::pair_t(0, cube::value_t::False)
           << cube::pair_t(1, cube::value_t::True)
           << cube::pair_t(2, cube::value_t::Dont_Care)
          ;
      }

      const cube c(c_file);
      AssertThat(c.dimensionality(), Is().EqualTo(1u));
    });


    it("reports dimensionality of { 0->0, 1->1, 2->2 } as 1", []() {
      adiar::shared_file<cube::pair_t> c_file;
      { adiar::file_writer<cube::pair_t> cw(c_file);
        cw << cube::pair_t(0, cube::value_t::Dont_Care)
           << cube::pair_t(1, cube::value_t::True)
           << cube::pair_t(2, cube::value_t::Dont_Care)
          ;
      }

      const cube c(c_file);
      AssertThat(c.dimensionality(), Is().EqualTo(2u));
    });

    describe("cube_union(const cube&, const cube&) (operator '|')", []() {
      it("computes Ø from Ø | Ø", []() {
        const cube a;
        const cube b;

        const cube res = a | b;

        AssertThat(res.empty(), Is().True());
      });

      it("computes { 0->0, 0->1 } from { 0->0, 0->1 } | Ø", []() {
        adiar::shared_file<cube::pair_t> a_file;
        { adiar::file_writer<cube::pair_t> aw(a_file);
          aw << cube::pair_t(0, cube::value_t::False)
             << cube::pair_t(1, cube::value_t::True);
        }
        const cube a(a_file); // <- { 0:0, 1:2 }

        const cube b; // <- Ø

        const cube res = a | b;

        cube_stream cs(res);

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(0, cube::value_t::False)));

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(1, cube::value_t::True)));

        AssertThat(cs.can_pull(), Is().False());
      });

      it("computes { 0->2, 0->1 } from Ø | { 0->0, 1->2 }", []() {
        const cube a; // <- Ø

        adiar::shared_file<cube::pair_t> b_file;
        { adiar::file_writer<cube::pair_t> bw(b_file);
          bw << cube::pair_t(0, cube::value_t::False)
             << cube::pair_t(1, cube::value_t::Dont_Care);
        }
        const cube b(b_file); // <- { 0:0, 1:2 }

        const cube res = a | b;

        cube_stream cs(res);

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(0, cube::value_t::False)));

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(1, cube::value_t::Dont_Care)));

        AssertThat(cs.can_pull(), Is().False());
      });

      it("computes { 0->2, 1->2, 2->0, 3->1 } from { 0->1, 1->1, 2->0 } | { 0->0, 1->2, 3->1 }", []() {
        adiar::shared_file<cube::pair_t> a_file;
        { adiar::file_writer<cube::pair_t> aw(a_file);
          aw << cube::pair_t(0, cube::value_t::True)
             << cube::pair_t(1, cube::value_t::True)
             << cube::pair_t(2, cube::value_t::False)
            ;
        }
        const cube a(a_file); // <- { 0:1, 1:1, 2:0 }

        adiar::shared_file<cube::pair_t> b_file;
        { adiar::file_writer<cube::pair_t> bw(b_file);
          bw << cube::pair_t(0, cube::value_t::False)
             << cube::pair_t(1, cube::value_t::Dont_Care)
             << cube::pair_t(3, cube::value_t::True)
            ;
        }
        const cube b(b_file); // <- { 0:0, 1:2, 3:1 }

        const cube res = a | b;

        cube_stream cs(res);

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(0, cube::value_t::Dont_Care)));

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(1, cube::value_t::Dont_Care)));

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(2, cube::value_t::False)));

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(3, cube::value_t::True)));

        AssertThat(cs.can_pull(), Is().False());
      });

      it("computes { 0->1, 1->2, 2->0, 3->1 } from { 0->1, 1->1, 3->1 } | { 1->0, 2->2, 3->1 }", []() {
        adiar::shared_file<cube::pair_t> a_file;
        { adiar::file_writer<cube::pair_t> aw(a_file);
          aw << cube::pair_t(0, cube::value_t::True)
             << cube::pair_t(1, cube::value_t::True)
             << cube::pair_t(3, cube::value_t::True)
            ;
        }
        const cube a(a_file); // <- { 0:1, 1:1, 2:0 }

        adiar::shared_file<cube::pair_t> b_file;
        { adiar::file_writer<cube::pair_t> bw(b_file);
          bw << cube::pair_t(1, cube::value_t::False)
             << cube::pair_t(2, cube::value_t::Dont_Care)
             << cube::pair_t(3, cube::value_t::True)
            ;
        }
        const cube b(b_file); // <- { 0:0, 1:2, 3:1 }

        const cube res = a | b;

        cube_stream cs(res);

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(0, cube::value_t::True)));

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(1, cube::value_t::Dont_Care)));

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(2, cube::value_t::Dont_Care)));

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(3, cube::value_t::True)));

        AssertThat(cs.can_pull(), Is().False());
      });

      it("can compute and assign (operator '|=')", []() {
        adiar::shared_file<cube::pair_t> a_file;
        { adiar::file_writer<cube::pair_t> aw(a_file);
          aw << cube::pair_t(0, cube::value_t::True)
             << cube::pair_t(1, cube::value_t::True)
             << cube::pair_t(3, cube::value_t::True)
            ;
        }
        cube a(a_file); // <- { 0:1, 1:1, 2:0 }

        adiar::shared_file<cube::pair_t> b_file;
        { adiar::file_writer<cube::pair_t> bw(b_file);
          bw << cube::pair_t(1, cube::value_t::False)
             << cube::pair_t(2, cube::value_t::Dont_Care)
             << cube::pair_t(3, cube::value_t::True)
            ;
        }
        const cube b(b_file); // <- { 0:0, 1:2, 3:1 }

        a |= b;

        cube_stream cs(a);

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(0, cube::value_t::True)));

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(1, cube::value_t::Dont_Care)));

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(2, cube::value_t::Dont_Care)));

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(3, cube::value_t::True)));

        AssertThat(cs.can_pull(), Is().False());
      });
    });

    describe("cube_intsec(const cube&, const cube&) (operator '&')", []() {
      it("computes Ø from Ø & Ø", []() {
        const cube a;
        const cube b;

        const cube res = a & b;
        AssertThat(res.empty(), Is().True());
      });

      it("computes Ø from { 0->2, 0->1 } & Ø", []() {
        adiar::shared_file<cube::pair_t> a_file;
        { adiar::file_writer<cube::pair_t> aw(a_file);
          aw << cube::pair_t(0, cube::value_t::Dont_Care)
             << cube::pair_t(1, cube::value_t::True);
        }
        const cube a(a_file); // <- { 0:0, 1:2 }

        const cube b; // <- Ø

        const cube res = a & b;
        AssertThat(res.empty(), Is().True());
      });

      it("computes Ø from Ø & { 0->0, 1->2 }", []() {
        const cube a; // <- Ø

        adiar::shared_file<cube::pair_t> b_file;
        { adiar::file_writer<cube::pair_t> bw(b_file);
          bw << cube::pair_t(0, cube::value_t::False)
             << cube::pair_t(1, cube::value_t::Dont_Care);
        }
        const cube b(b_file); // <- { 0:0, 1:2 }

        const cube res = a & b;
        AssertThat(res.empty(), Is().True());
      });

      it("computes { 1->1 } from { 0->1, 1->1 } & { 0->0, 1->1 }", []() {
        adiar::shared_file<cube::pair_t> a_file;
        { adiar::file_writer<cube::pair_t> aw(a_file);
          aw << cube::pair_t(0, cube::value_t::True)
             << cube::pair_t(1, cube::value_t::True)
            ;
        }
        const cube a(a_file); // <- { 0:1, 1:1, 2:0 }

        adiar::shared_file<cube::pair_t> b_file;
        { adiar::file_writer<cube::pair_t> bw(b_file);
          bw << cube::pair_t(0, cube::value_t::False)
             << cube::pair_t(1, cube::value_t::True)
            ;
        }
        const cube b(b_file); // <- { 0:0, 1:2, 3:1 }

        const cube res = a & b;

        cube_stream cs(res);

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(1, cube::value_t::True)));

        AssertThat(cs.can_pull(), Is().False());
      });

      it("computes { 0->1, 1->2 } from { 0->1, 1->1 } & { 0->2, 1->2 }", []() {
        adiar::shared_file<cube::pair_t> a_file;
        { adiar::file_writer<cube::pair_t> aw(a_file);
          aw << cube::pair_t(0, cube::value_t::True)
             << cube::pair_t(1, cube::value_t::Dont_Care)
            ;
        }
        const cube a(a_file); // <- { 0:1, 1:1, 2:0 }

        adiar::shared_file<cube::pair_t> b_file;
        { adiar::file_writer<cube::pair_t> bw(b_file);
          bw << cube::pair_t(0, cube::value_t::Dont_Care)
             << cube::pair_t(1, cube::value_t::Dont_Care)
            ;
        }
        const cube b(b_file); // <- { 0:0, 1:2, 3:1 }

        const cube res = a & b;

        cube_stream cs(res);

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(0, cube::value_t::True)));

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(1, cube::value_t::Dont_Care)));

        AssertThat(cs.can_pull(), Is().False());
      });

      it("computes { 1->2 } from { 0->1, 1->2 } & { 1->2, 2->2 }", []() {
        adiar::shared_file<cube::pair_t> a_file;
        { adiar::file_writer<cube::pair_t> aw(a_file);
          aw << cube::pair_t(0, cube::value_t::True)
             << cube::pair_t(1, cube::value_t::Dont_Care)
            ;
        }
        const cube a(a_file); // <- { 0:1, 1:1, 2:0 }

        adiar::shared_file<cube::pair_t> b_file;
        { adiar::file_writer<cube::pair_t> bw(b_file);
          bw << cube::pair_t(1, cube::value_t::Dont_Care)
             << cube::pair_t(2, cube::value_t::Dont_Care)
            ;
        }
        const cube b(b_file); // <- { 0:0, 1:2, 3:1 }

        const cube res = a & b;

        cube_stream cs(res);

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(1, cube::value_t::Dont_Care)));

        AssertThat(cs.can_pull(), Is().False());
      });

      it("can compute and assign (operator '&=')", []() {
        adiar::shared_file<cube::pair_t> a_file;
        { adiar::file_writer<cube::pair_t> aw(a_file);
          aw << cube::pair_t(0, cube::value_t::True)
             << cube::pair_t(1, cube::value_t::Dont_Care)
            ;
        }
        cube a(a_file); // <- { 0:1, 1:1, 2:0 }

        adiar::shared_file<cube::pair_t> b_file;
        { adiar::file_writer<cube::pair_t> bw(b_file);
          bw << cube::pair_t(1, cube::value_t::Dont_Care)
             << cube::pair_t(2, cube::value_t::Dont_Care)
            ;
        }
        const cube b(b_file); // <- { 0:0, 1:2, 3:1 }

        a &= b;

        cube_stream cs(a);

        AssertThat(cs.can_pull(), Is().True());
        AssertThat(cs.pull(), Is().EqualTo(cube::pair_t(1, cube::value_t::Dont_Care)));

        AssertThat(cs.can_pull(), Is().False());
      });
    });
  });
 });

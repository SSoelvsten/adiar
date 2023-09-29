#include "../test.h"

go_bandit([]() {
  describe("adiar/exec_policy.h", []() {
    describe("exec_policy", []() {
      describe("set(const __ &)", []() {
        it("is default constructed with default settings", []() {
          exec_policy ep;

          AssertThat(ep.access_mode(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.memory_mode(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Auto));
        });

        it("can be conversion constructed from 'access mode'", []() {
          exec_policy ep = exec_policy::access::Priority_Queue;

          AssertThat(ep.access_mode(),  Is().EqualTo(exec_policy::access::Priority_Queue));
          AssertThat(ep.memory_mode(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Auto));
        });

        it("can be conversion constructed from 'memory mode'", []() {
          exec_policy ep = exec_policy::memory::Internal;

          AssertThat(ep.access_mode(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.memory_mode(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Auto));
        });

        it("can be conversion constructed from 'quantify algorithm'", []() {
          exec_policy ep = exec_policy::quantify::Nested;

          AssertThat(ep.access_mode(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.memory_mode(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Nested));
        });
      });

      describe("set(const __ &)", []() {
        it("can set 'access mode'", []() {
          exec_policy ep;
          AssertThat(ep.access_mode(), Is().EqualTo(exec_policy::access::Auto));

          ep.set(exec_policy::access::Random_Access);
          AssertThat(ep.access_mode(), Is().EqualTo(exec_policy::access::Random_Access));

          ep.set(exec_policy::access::Priority_Queue);
          AssertThat(ep.access_mode(), Is().EqualTo(exec_policy::access::Priority_Queue));

          ep.set(exec_policy::access::Auto);
          AssertThat(ep.access_mode(), Is().EqualTo(exec_policy::access::Auto));
        });

        it("can set 'memory mode'", []() {
          exec_policy ep;
          AssertThat(ep.memory_mode(), Is().EqualTo(exec_policy::memory::Auto));

          ep.set(exec_policy::memory::Internal);
          AssertThat(ep.memory_mode(), Is().EqualTo(exec_policy::memory::Internal));

          ep.set(exec_policy::memory::External);
          AssertThat(ep.memory_mode(), Is().EqualTo(exec_policy::memory::External));

          ep.set(exec_policy::memory::Auto);
          AssertThat(ep.memory_mode(), Is().EqualTo(exec_policy::memory::Auto));
        });

        it("can set 'quantify algorithm'", []() {
          exec_policy ep;
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Auto));

          ep.set(exec_policy::quantify::Nested);
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Nested));

          ep.set(exec_policy::quantify::Partial);
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Partial));

          ep.set(exec_policy::quantify::Singleton);
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Singleton));

          ep.set(exec_policy::quantify::Auto);
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Auto));
        });

        it("can set settigs akin to Adiar v1.0", []() {
          exec_policy ep;

          ep.set(exec_policy::access::Priority_Queue);
          ep.set(exec_policy::memory::External);
          ep.set(exec_policy::quantify::Singleton);

          AssertThat(ep.access_mode(),  Is().EqualTo(exec_policy::access::Priority_Queue));
          AssertThat(ep.memory_mode(),  Is().EqualTo(exec_policy::memory::External));
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Singleton));
        });

        it("can set settigs with a builder pattern syntax", []() {
          exec_policy ep;

          ep.set(exec_policy::access::Priority_Queue)
            .set(exec_policy::memory::External)
            .set(exec_policy::quantify::Singleton);

          AssertThat(ep.access_mode(), Is().EqualTo(exec_policy::access::Priority_Queue));
          AssertThat(ep.memory_mode(), Is().EqualTo(exec_policy::memory::External));
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Singleton));
        });
      });

      describe("operator &(const exec_policy&)", []() {
        it("can create a copy with another 'access mode'", []() {
          exec_policy in = exec_policy::memory::Internal;
          exec_policy out = in & exec_policy::access::Random_Access;

          AssertThat(in.access_mode(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(in.memory_mode(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(in.quantify_alg(), Is().EqualTo(exec_policy::quantify::Auto));

          AssertThat(out.access_mode(),  Is().EqualTo(exec_policy::access::Random_Access));
          AssertThat(out.memory_mode(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(out.quantify_alg(), Is().EqualTo(exec_policy::quantify::Auto));
        });

        it("can create a copy with another 'memory mode'", []() {
          exec_policy in = exec_policy::access::Priority_Queue;
          exec_policy out = in & exec_policy::memory::Internal;

          AssertThat(in.access_mode(),  Is().EqualTo(exec_policy::access::Priority_Queue));
          AssertThat(in.memory_mode(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(in.quantify_alg(), Is().EqualTo(exec_policy::quantify::Auto));

          AssertThat(out.access_mode(),  Is().EqualTo(exec_policy::access::Priority_Queue));
          AssertThat(out.memory_mode(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(out.quantify_alg(), Is().EqualTo(exec_policy::quantify::Auto));
        });

        it("can create a copy with another 'quantify algorithm'", []() {
          exec_policy in = exec_policy::memory::Internal;
          exec_policy out = in & exec_policy::quantify::Partial;

          AssertThat(in.access_mode(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(in.memory_mode(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(in.quantify_alg(), Is().EqualTo(exec_policy::quantify::Auto));

          AssertThat(out.access_mode(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(out.memory_mode(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(out.quantify_alg(), Is().EqualTo(exec_policy::quantify::Partial));
        });

        it("can lift enum values [access & memory]", []() {
          exec_policy ep = exec_policy::access::Random_Access & exec_policy::memory::Internal;

          AssertThat(ep.access_mode(),  Is().EqualTo(exec_policy::access::Random_Access));
          AssertThat(ep.memory_mode(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Auto));
        });

        it("can lift enum values [memory & access]", []() {
          exec_policy ep = exec_policy::memory::Internal & exec_policy::access::Random_Access;

          AssertThat(ep.access_mode(),  Is().EqualTo(exec_policy::access::Random_Access));
          AssertThat(ep.memory_mode(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Auto));
        });

        it("can lift enum values [access & quantify]", []() {
          exec_policy ep = exec_policy::access::Random_Access & exec_policy::quantify::Singleton;

          AssertThat(ep.access_mode(),  Is().EqualTo(exec_policy::access::Random_Access));
          AssertThat(ep.memory_mode(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Singleton));
        });

        it("can lift enum values [quantify & access]", []() {
          exec_policy ep = exec_policy::quantify::Singleton & exec_policy::access::Random_Access;

          AssertThat(ep.access_mode(), Is().EqualTo(exec_policy::access::Random_Access));
          AssertThat(ep.memory_mode(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Singleton));
        });

        it("can lift enum values [memory & quantify]", []() {
          exec_policy ep = exec_policy::memory::External & exec_policy::quantify::Nested;

          AssertThat(ep.access_mode(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.memory_mode(),  Is().EqualTo(exec_policy::memory::External));
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Nested));
        });

        it("can lift enum values [quantify & access]", []() {
          exec_policy ep = exec_policy::quantify::Nested & exec_policy::memory::External;

          AssertThat(ep.access_mode(), Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.memory_mode(),  Is().EqualTo(exec_policy::memory::External));
          AssertThat(ep.quantify_alg(), Is().EqualTo(exec_policy::quantify::Nested));
        });
      });
    });
  });
 });

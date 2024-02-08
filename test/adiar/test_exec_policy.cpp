#include "../test.h"

go_bandit([]() {
  describe("adiar/exec_policy.h", []() {
    describe("exec_policy", []() {
      it("uses expected number of bytes", []() {
        AssertThat(sizeof(exec_policy),  Is().EqualTo(4u));
      });

      describe("exec_policy(const __ &)", []() {
        it("is default constructed with default settings", []() {
          exec_policy ep;

          AssertThat(ep.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(-1.0));
        });

        it("can be conversion constructed from 'access mode'", []() {
          exec_policy ep = exec_policy::access::Priority_Queue;

          AssertThat(ep.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Priority_Queue));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(-1.0));
        });

        it("can be conversion constructed from 'memory mode'", []() {
          exec_policy ep = exec_policy::memory::Internal;

          AssertThat(ep.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(-1.0));
        });

        it("can be conversion constructed from 'quantify algorithm'", []() {
          exec_policy ep = exec_policy::quantify::Nested;

          AssertThat(ep.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Nested));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(-1.0));
        });
      });

      describe("set(const __ &)", []() {
        it("can set 'access mode'", []() {
          exec_policy ep;
          AssertThat(ep.template get<exec_policy::access>(), Is().EqualTo(exec_policy::access::Auto));

          ep.set(exec_policy::access::Random_Access);
          AssertThat(ep.template get<exec_policy::access>(), Is().EqualTo(exec_policy::access::Random_Access));

          ep.set(exec_policy::access::Priority_Queue);
          AssertThat(ep.template get<exec_policy::access>(), Is().EqualTo(exec_policy::access::Priority_Queue));

          ep.set(exec_policy::access::Auto);
          AssertThat(ep.template get<exec_policy::access>(), Is().EqualTo(exec_policy::access::Auto));
        });

        it("can set 'memory mode'", []() {
          exec_policy ep;
          AssertThat(ep.template get<exec_policy::memory>(), Is().EqualTo(exec_policy::memory::Auto));

          ep.set(exec_policy::memory::Internal);
          AssertThat(ep.template get<exec_policy::memory>(), Is().EqualTo(exec_policy::memory::Internal));

          ep.set(exec_policy::memory::External);
          AssertThat(ep.template get<exec_policy::memory>(), Is().EqualTo(exec_policy::memory::External));

          ep.set(exec_policy::memory::Auto);
          AssertThat(ep.template get<exec_policy::memory>(), Is().EqualTo(exec_policy::memory::Auto));
        });

        it("can set 'quantify algorithm'", []() {
          exec_policy ep;
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));

          ep.set(exec_policy::quantify::Nested);
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Nested));

          ep.set(exec_policy::quantify::Partial);
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Partial));

          ep.set(exec_policy::quantify::Singleton);
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Singleton));

          ep.set(exec_policy::quantify::Auto);
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));
        });

        it("can set 'nested::fast reduce epsilon'", []() {
          exec_policy ep;
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(-1.0));

          ep.set(exec_policy::nested::fast_reduce(0.0));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(0.0));

          ep.set(exec_policy::nested::fast_reduce(1.0));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(1.0));

          ep.set(exec_policy::nested::fast_reduce(0.05));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().GreaterThan(0.04));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().LessThan(0.06));

          ep.set(exec_policy::nested::fast_reduce(0.5));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().GreaterThan(0.49));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().LessThan(0.51));
        });

        it("can set settigs with a builder pattern syntax", []() {
          exec_policy ep;

          ep.set(exec_policy::access::Priority_Queue)
            .set(exec_policy::memory::External)
            .set(exec_policy::quantify::Singleton)
            .set(exec_policy::nested::fast_reduce(1.0));

          AssertThat(ep.template get<exec_policy::access>(), Is().EqualTo(exec_policy::access::Priority_Queue));
          AssertThat(ep.template get<exec_policy::memory>(), Is().EqualTo(exec_policy::memory::External));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Singleton));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(1.0));
        });
      });

      describe("operator ==(const exec_policy&)", []() {
        it("matches for default settings", []() {
          exec_policy ep1;
          exec_policy ep2;

          AssertThat(ep1, Is().EqualTo(ep2));
        });

        it("matches for (non-default) Adiar v1.0 settings", []() {
          exec_policy ep1;
          ep1.set(exec_policy::quantify::Singleton)
             .set(exec_policy::memory::External)
             .set(exec_policy::access::Priority_Queue);

          exec_policy ep2;
          ep2.set(exec_policy::access::Priority_Queue)
             .set(exec_policy::memory::External)
             .set(exec_policy::quantify::Singleton);

          AssertThat(ep1, Is().EqualTo(ep2));
        });

        it("mismatches on 'access mode'", []() {
          exec_policy ep1 = exec_policy::access::Priority_Queue;
          exec_policy ep2 = exec_policy::access::Random_Access;

          AssertThat(ep1, Is().Not().EqualTo(ep2));
        });

        it("mismatches on 'memory mode'", []() {
          exec_policy ep1 = exec_policy::memory::Internal;
          exec_policy ep2 = exec_policy::memory::Auto;

          AssertThat(ep1, Is().Not().EqualTo(ep2));
        });

        it("mismatches on 'quantify algorithm'", []() {
          exec_policy ep1 = exec_policy::quantify::Nested;
          exec_policy ep2 = exec_policy::quantify::Partial;

          AssertThat(ep1, Is().Not().EqualTo(ep2));
        });

        it("mismatches on 'nested::fast reduce epsilon'", []() {
          exec_policy ep1 = exec_policy::nested::fast_reduce(0.0);
          exec_policy ep2 = exec_policy::nested::fast_reduce(1.0);

          AssertThat(ep1, Is().Not().EqualTo(ep2));
        });
      });

      describe("operator &(const exec_policy&)", []() {
        it("can create a copy with another 'access mode'", []() {
          const exec_policy in = exec_policy::memory::Internal;
          const exec_policy out = in & exec_policy::access::Random_Access;

          AssertThat(in.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(in.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(in.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));

          AssertThat(out.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Random_Access));
          AssertThat(out.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(out.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));
        });

        it("can create a copy with another 'memory mode'", []() {
          const exec_policy in = exec_policy::access::Priority_Queue;
          const exec_policy out = in & exec_policy::memory::Internal;

          AssertThat(in.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Priority_Queue));
          AssertThat(in.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(in.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));

          AssertThat(out.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Priority_Queue));
          AssertThat(out.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(out.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));
        });

        it("can create a copy with another 'quantify algorithm'", []() {
          const exec_policy in = exec_policy::memory::Internal;
          const exec_policy out = in & exec_policy::quantify::Partial;

          AssertThat(in.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(in.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(in.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));

          AssertThat(out.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(out.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(out.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Partial));
        });

        it("can create a copy with another 'nested::fast reduce epsilon'", []() {
          const exec_policy in = exec_policy::memory::Internal;
          const exec_policy out = in & exec_policy::nested::fast_reduce(1.0);

          AssertThat(in.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(in.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(in.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));
          AssertThat(static_cast<float>(in.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(-1.0));

          AssertThat(out.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(out.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(out.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));
          AssertThat(static_cast<float>(out.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(1.0));
        });

        it("can lift enum values [access & memory]", []() {
          const exec_policy ep = exec_policy::access::Random_Access & exec_policy::memory::Internal;

          AssertThat(ep.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Random_Access));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(-1.0));
        });

        it("can lift enum values [memory & access]", []() {
          const exec_policy ep = exec_policy::memory::Internal & exec_policy::access::Random_Access;

          AssertThat(ep.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Random_Access));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(-1.0));
        });

        it("can lift enum values [access & quantify]", []() {
          const exec_policy ep = exec_policy::access::Random_Access & exec_policy::quantify::Singleton;

          AssertThat(ep.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Random_Access));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Singleton));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(-1.0));
        });

        it("can lift enum values [quantify & access]", []() {
          const exec_policy ep = exec_policy::quantify::Singleton & exec_policy::access::Random_Access;

          AssertThat(ep.template get<exec_policy::access>(), Is().EqualTo(exec_policy::access::Random_Access));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Singleton));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(-1.0));
        });

        it("can lift enum values [memory & quantify]", []() {
          const exec_policy ep = exec_policy::memory::External & exec_policy::quantify::Nested;

          AssertThat(ep.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::External));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Nested));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(-1.0));
        });

        it("can lift enum values [quantify & memory]", []() {
          const exec_policy ep = exec_policy::quantify::Nested & exec_policy::memory::External;

          AssertThat(ep.template get<exec_policy::access>(), Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::External));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Nested));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(-1.0));
        });

        it("can lift enum values [access & nested::fast reduce epsilon]", []() {
          const exec_policy ep = exec_policy::access::Random_Access & exec_policy::nested::fast_reduce(1.0);

          AssertThat(ep.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Random_Access));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(1.0));
        });

        it("can lift enum values [nested::fast reduce epsilon & access]", []() {
          const exec_policy ep = exec_policy::nested::fast_reduce(1.0) & exec_policy::access::Random_Access;

          AssertThat(ep.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Random_Access));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(1.0));
        });

        it("can lift enum values [memory & nested::fast reduce epsilon]", []() {
          const exec_policy ep = exec_policy::memory::Internal & exec_policy::nested::fast_reduce(1.0);

          AssertThat(ep.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Internal));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(1.0));
        });

        it("can lift enum values [nested::fast reduce epsilon & memory]", []() {
          const exec_policy ep = exec_policy::nested::fast_reduce(1.0) & exec_policy::memory::External;

          AssertThat(ep.template get<exec_policy::access>(), Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::External));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Auto));
          AssertThat(static_cast<float>(ep.template get<exec_policy::nested::fast_reduce>()),
                     Is().EqualTo(1.0));
        });

        it("can lift enum values [quantify & nested::fast reduce epsilon]", []() {
          const exec_policy ep = exec_policy::quantify::Singleton & exec_policy::nested::fast_reduce(1.0);

          AssertThat(ep.template get<exec_policy::access>(),  Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Singleton));
        });

        it("can lift enum values [nested::fast reduce epsilon & quantify]", []() {
          const exec_policy ep = exec_policy::nested::fast_reduce(1.0) & exec_policy::quantify::Nested;

          AssertThat(ep.template get<exec_policy::access>(), Is().EqualTo(exec_policy::access::Auto));
          AssertThat(ep.template get<exec_policy::memory>(),  Is().EqualTo(exec_policy::memory::Auto));
          AssertThat(ep.template get<exec_policy::quantify>(), Is().EqualTo(exec_policy::quantify::Nested));
        });
      });
    });
  });
 });

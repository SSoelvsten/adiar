#include "../../../test.h"

#include <array>
#include <filesystem>

namespace adiar::internal
{
  template <>
  struct FILE_CONSTANTS<int>
  {
    static constexpr size_t files = 2u;

    struct stats
    { /* No extra 'int' specific variables */ };
  };
}

go_bandit([]() {
  describe("adiar/internal/io/levelized_file.h , levelized_file_stream.h, levelized_file_writer.h", []() {
    it("levelized_file::FILES exposes the FILE_CONSTANT", []() {
      AssertThat(__levelized_file<int>::FILES, Is().EqualTo(2u));
    });

    describe("levelized_file() [empty]", []() {
      it("creates a file marked as 'temporary'", []() {
        __levelized_file<int> lf;
        AssertThat(lf.is_persistent(), Is().False());
        AssertThat(lf.is_temp(), Is().True());
      });

      it("has no content", []() {
        __levelized_file<int> lf;
        AssertThat(lf.size(), Is().EqualTo(0u));
        AssertThat(lf.empty(), Is().True());
      });

      it("has no levels", []() {
        __levelized_file<int> lf;
        AssertThat(lf.levels(), Is().EqualTo(0u));
      });

      it("has FILES+1 different paths", []() {
        __levelized_file<int> lf;
        auto paths = lf.paths();

        // Check array has expected size.
        AssertThat(paths.size(), Is().EqualTo(2u + 1u));

        // Check array has non-trivial values
        AssertThat(paths[0], Is().Not().EqualTo(""));
        AssertThat(paths[1], Is().Not().EqualTo(""));
        AssertThat(paths[2], Is().Not().EqualTo(""));

        // Check all are different
        AssertThat(paths[0], Is().Not().EqualTo(paths[1]));
        AssertThat(paths[0], Is().Not().EqualTo(paths[2]));
        AssertThat(paths[1], Is().Not().EqualTo(paths[2]));
      });

      it("does not (yet) 'exist' on disk", []() {
        __levelized_file<int> lf;
        AssertThat(lf.exists(), Is().False());

        auto paths = lf.paths();
        AssertThat(std::filesystem::exists(paths[0]), Is().False());
        AssertThat(std::filesystem::exists(paths[1]), Is().False());
        AssertThat(std::filesystem::exists(paths[2]), Is().False());
      });

      it("'exists' after a 'touch'", []() {
        __levelized_file<int> lf;
        AssertThat(lf.exists(), Is().False());

        auto paths = lf.paths();

        lf.touch();
        AssertThat(lf.exists(), Is().True());

        AssertThat(std::filesystem::exists(paths[0]), Is().True());
        AssertThat(std::filesystem::exists(paths[1]), Is().True());
        AssertThat(std::filesystem::exists(paths[2]), Is().True());
      });

      it("is a fresh file", []() {
        __levelized_file<int> lf1;
        auto paths1 = lf1.paths();

        __levelized_file<int> lf2;
        auto paths2 = lf2.paths();

        AssertThat(paths1[0], Is().Not().EqualTo(paths2[0]));
        AssertThat(paths1[0], Is().Not().EqualTo(paths2[1]));
        AssertThat(paths1[0], Is().Not().EqualTo(paths2[2]));

        AssertThat(paths1[1], Is().Not().EqualTo(paths2[0]));
        AssertThat(paths1[1], Is().Not().EqualTo(paths2[1]));
        AssertThat(paths1[1], Is().Not().EqualTo(paths2[2]));

        AssertThat(paths1[2], Is().Not().EqualTo(paths2[0]));
        AssertThat(paths1[2], Is().Not().EqualTo(paths2[1]));
        AssertThat(paths1[2], Is().Not().EqualTo(paths2[2]));
      });

      it("is temporary (removed from disk after destruct)", []() {
        std::array<std::string, 2u + 1u> paths;
        {
          __levelized_file<int> lf;
          paths = lf.paths();
          lf.touch();

          AssertThat(std::filesystem::exists(paths[0]), Is().True());
          AssertThat(std::filesystem::exists(paths[1]), Is().True());
          AssertThat(std::filesystem::exists(paths[2]), Is().True());
        }
        AssertThat(std::filesystem::exists(paths[0]), Is().False());
        AssertThat(std::filesystem::exists(paths[1]), Is().False());
        AssertThat(std::filesystem::exists(paths[2]), Is().False());
      });

      it("can be 'moved' when not existing", []() {
        std::string new_path_prefix = "./after-move-path.adiar";

        std::array<std::string, 2u+1u> new_paths = {
          new_path_prefix+".file_0",
          new_path_prefix+".file_1",
          new_path_prefix+".levels"
        };

        // Clean up after prior test run
        for (const std::string &path : new_paths) {
          if (std::filesystem::exists(path))
            std::filesystem::remove(path);
        }

        __levelized_file<int> lf;
        std::array<std::string, 2u+1u> old_paths = lf.paths();

        AssertThat(std::filesystem::exists(old_paths[0]), Is().False());
        AssertThat(std::filesystem::exists(old_paths[1]), Is().False());
        AssertThat(std::filesystem::exists(old_paths[2]), Is().False());

        AssertThat(std::filesystem::exists(new_paths[1]), Is().False());
        AssertThat(std::filesystem::exists(new_paths[1]), Is().False());
        AssertThat(std::filesystem::exists(new_paths[2]), Is().False());

        lf.move(new_path_prefix);

        AssertThat(std::filesystem::exists(old_paths[0]), Is().False());
        AssertThat(std::filesystem::exists(old_paths[1]), Is().False());
        AssertThat(std::filesystem::exists(old_paths[2]), Is().False());

        AssertThat(std::filesystem::exists(new_paths[1]), Is().False());
        AssertThat(std::filesystem::exists(new_paths[1]), Is().False());
        AssertThat(std::filesystem::exists(new_paths[2]), Is().False());

        lf.touch();

        AssertThat(std::filesystem::exists(old_paths[0]), Is().False());
        AssertThat(std::filesystem::exists(old_paths[1]), Is().False());
        AssertThat(std::filesystem::exists(old_paths[2]), Is().False());

        AssertThat(std::filesystem::exists(new_paths[1]), Is().True());
        AssertThat(std::filesystem::exists(new_paths[1]), Is().True());
        AssertThat(std::filesystem::exists(new_paths[2]), Is().True());
      });

      it("can be 'moved' when existing", []() {
        std::string new_path_prefix = "./after-move-path.adiar";

        std::array<std::string, 2u+1u> new_paths = {
          new_path_prefix+".file_0",
          new_path_prefix+".file_1",
          new_path_prefix+".levels"
        };

        // Clean up after prior test run
        for (const std::string &path : new_paths) {
          if (std::filesystem::exists(path))
            std::filesystem::remove(path);
        }

        __levelized_file<int> lf;
        lf.touch();
        std::array<std::string, 2u+1u> old_paths = lf.paths();

        AssertThat(std::filesystem::exists(old_paths[0]), Is().True());
        AssertThat(std::filesystem::exists(old_paths[1]), Is().True());
        AssertThat(std::filesystem::exists(old_paths[2]), Is().True());

        AssertThat(std::filesystem::exists(new_paths[1]), Is().False());
        AssertThat(std::filesystem::exists(new_paths[1]), Is().False());
        AssertThat(std::filesystem::exists(new_paths[2]), Is().False());

        lf.move(new_path_prefix);

        AssertThat(std::filesystem::exists(old_paths[0]), Is().False());
        AssertThat(std::filesystem::exists(old_paths[1]), Is().False());
        AssertThat(std::filesystem::exists(old_paths[2]), Is().False());

        AssertThat(std::filesystem::exists(new_paths[1]), Is().True());
        AssertThat(std::filesystem::exists(new_paths[1]), Is().True());
        AssertThat(std::filesystem::exists(new_paths[2]), Is().True());
      });

      it("is still temporary after move", []() {
        std::string new_path_prefix = "./after-move-path.adiar";

        std::array<std::string, 2u+1u> new_paths = {
          new_path_prefix+".file_0",
          new_path_prefix+".file_1",
          new_path_prefix+".levels"
        };

        // Clean up after prior test run
        for (const std::string &path : new_paths) {
          if (std::filesystem::exists(path))
            std::filesystem::remove(path);
        }

        {
          __levelized_file<int> lf;
          lf.touch();
          lf.move(new_path_prefix);

          AssertThat(std::filesystem::exists(new_paths[1]), Is().True());
          AssertThat(std::filesystem::exists(new_paths[1]), Is().True());
          AssertThat(std::filesystem::exists(new_paths[2]), Is().True());

          AssertThat(lf.is_persistent(), Is().False());
          AssertThat(lf.is_temp(), Is().True());
        }

        AssertThat(std::filesystem::exists(new_paths[1]), Is().False());
        AssertThat(std::filesystem::exists(new_paths[1]), Is().False());
        AssertThat(std::filesystem::exists(new_paths[2]), Is().False());
      });

      it("cannot be 'moved' on-top of another (non-empty) file", []() {
        std::string new_path_prefix = "./after-move-path.adiar";

        std::array<std::string, 2u+1u> new_paths = {
          new_path_prefix+".file_0",
          new_path_prefix+".file_1",
          new_path_prefix+".levels"
        };

        // Clean up after prior test run
        for (const std::string &path : new_paths) {
          if (std::filesystem::exists(path))
            std::filesystem::remove(path);
        }


        __levelized_file<int> lf1;
        lf1.move(new_path_prefix);
        lf1.touch();

        __levelized_file<int> lf2;

        AssertThrows(std::runtime_error, lf2.move(new_path_prefix));
      });

      it("can be made persistent (not removed from disk)", []() {
        std::array<std::string, 2u + 1u> paths;
        {
          __levelized_file<int> lf;
          paths = lf.paths();
          lf.touch();
          lf.make_persistent();

          AssertThat(lf.is_persistent(), Is().True());
          AssertThat(lf.is_temp(), Is().False());
        }
        AssertThat(std::filesystem::exists(paths[0]), Is().True());
        AssertThat(std::filesystem::exists(paths[1]), Is().True());
        AssertThat(std::filesystem::exists(paths[2]), Is().True());

        // Clean up for this test
        for (const std::string &path : paths) {
          if (std::filesystem::exists(path)) std::filesystem::remove(path);
        }
      });

      it("exists on disk after being made persistent", []() {
        __levelized_file<int> lf;
        std::array<std::string, 2u + 1u> paths = lf.paths();

        AssertThat(std::filesystem::exists(paths[0]), Is().False());
        AssertThat(std::filesystem::exists(paths[1]), Is().False());
        AssertThat(std::filesystem::exists(paths[2]), Is().False());

        lf.make_persistent();

        AssertThat(std::filesystem::exists(paths[0]), Is().True());
        AssertThat(std::filesystem::exists(paths[1]), Is().True());
        AssertThat(std::filesystem::exists(paths[2]), Is().True());

        // Clean up for this test
        for (const std::string &path : paths) {
          if (std::filesystem::exists(path)) std::filesystem::remove(path);
        }
      });

      it("cannot 'move' file marked persisted", []() {
        std::string new_path_prefix = "./after-move-path.adiar";

        std::array<std::string, 2u+1u> new_paths = {
          new_path_prefix+".file_0",
          new_path_prefix+".file_1",
          new_path_prefix+".levels"
        };

        // Clean up after prior test run
        for (const std::string &path : new_paths) {
          if (std::filesystem::exists(path))
            std::filesystem::remove(path);
        }

        __levelized_file<int> lf;
        lf.make_persistent();
        AssertThrows(std::runtime_error, lf.move(new_path_prefix));
      });
    });
  });
 });

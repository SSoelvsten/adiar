#include "../../../test.h"

#include <filesystem>

go_bandit([]() {
    describe("adiar/file.h", []() {
        describe("file() [empty]", []() {
            std::string path;

            it("creates a file marked as 'temporary'", []() {
                file<int> f;
                AssertThat(f.is_persistent(), Is().False());
                AssertThat(f.is_temp(), Is().True());
              });

            it("has no content", []() {
                file<int> f;
                AssertThat(f.size(), Is().EqualTo(0u));
                AssertThat(f.empty(), Is().True());
              });

            it("does not (yet) 'exist' on disk", [&path]() {
                file<int> f;
                AssertThat(f.exists(), Is().False());

                path = f.path();
                AssertThat(std::filesystem::exists(path), Is().False());
              });

            it("'exists' after a 'touch'", [&path]() {
                file<int> f;
                f.touch();
                AssertThat(f.exists(), Is().True());

                path = f.path();
                AssertThat(std::filesystem::exists(path), Is().True());
              });

            it("is a fresh file", [&path]() {
                file<int> f;
                f.touch();
                AssertThat(f.path(), Is().Not().EqualTo(path));
              });

            it("is temporary (removed from disk after destruct)", [&path]() {
                { // Scope to destruct 'f' early
                  file<int> f;
                  f.touch();

                  path = f.path();
                  AssertThat(std::filesystem::exists(path), Is().True());
                }
                AssertThat(std::filesystem::exists(path), Is().False());
              });

            it("can be 'moved' when empty", [&path]() {
                std::string new_path = "./after-move-path.adiar";
                if (std::filesystem::exists(new_path)) {
                  // Clean up after prior test run
                  std::filesystem::remove(new_path);
                }

                file<int> f;
                std::string old_path = f.path();

                AssertThat(std::filesystem::exists(old_path), Is().False());
                AssertThat(std::filesystem::exists(new_path), Is().False());

                f.move(new_path);

                AssertThat(std::filesystem::exists(old_path), Is().False());
                AssertThat(std::filesystem::exists(new_path), Is().False());

                f.touch();

                AssertThat(std::filesystem::exists(old_path), Is().False());
                AssertThat(std::filesystem::exists(new_path), Is().True());
              });

            it("can be 'moved' when non-empty", [&path]() {
                std::string new_path = "./after-move-path.adiar";
                if (std::filesystem::exists(new_path)) {
                  // Clean up after prior test run
                  std::filesystem::remove(new_path);
                }

                file<int> f;
                f.touch();
                std::string old_path = f.path();

                AssertThat(std::filesystem::exists(old_path), Is().True());
                AssertThat(std::filesystem::exists(new_path), Is().False());

                f.move(new_path);

                AssertThat(std::filesystem::exists(old_path), Is().False());
                AssertThat(std::filesystem::exists(new_path), Is().True());
              });

            it("cannot be 'moved' on-top of another (non-empty) file", [&path]() {
                file<int> f1;
                f1.touch();

                file<int> f2;
                f2.touch();

                AssertThrows(std::runtime_error, f1.move(f2.path()));
              });

            it("is still temporary after move", [&path]() {
                std::string new_path = "./after-move-path.adiar";
                if (std::filesystem::exists(new_path)) {
                  // Clean up after prior test run
                  std::filesystem::remove(new_path);
                }

                {
                  file<int> f;
                  f.touch();
                  std::string old_path = f.path();

                  f.move(new_path);

                  AssertThat(std::filesystem::exists(new_path), Is().True());

                  AssertThat(f.is_persistent(), Is().False());
                  AssertThat(f.is_temp(), Is().True());
                }
                AssertThat(std::filesystem::exists(new_path), Is().False());
              });

            it("can be made persistent (not removed from disk)", [&path]() {
                { // Scope to destruct 'f' early
                  file<int> f;
                  f.touch();

                  path = f.path();
                  AssertThat(std::filesystem::exists(path), Is().True());

                  f.make_persistent();
                  AssertThat(f.is_persistent(), Is().True());
                  AssertThat(f.is_temp(), Is().False());
                }
                AssertThat(std::filesystem::exists(path), Is().True());

                // Clean up for this test
                std::filesystem::remove(path);
              });

            it("exists on disk after being made persistent", [&path]() {
                { // Scope to destruct 'f' early
                  file<int> f;
                  path = f.path();

                  AssertThat(f.exists(), Is().False());
                  AssertThat(std::filesystem::exists(path), Is().False());
                  f.make_persistent();
                  AssertThat(f.exists(), Is().True());
                  AssertThat(std::filesystem::exists(path), Is().True());
                }
                // Clean up for this test
                if (std::filesystem::exists(path)) {
                  std::filesystem::remove(path);
                }
              });
          });

        describe("file(path) [empty]", []() {
            it("throws exception on path to non-existing file", []() {
                AssertThrows(std::runtime_error,
                             file<int>("./non-existing-file.adiar"));
              });

            std::string path;
            {
              file<int> f;
              f.touch();

              path = f.path();
              f.make_persistent();
            }

            it("can reopen existing file", [&path]() {
                file<int> f(path);
                AssertThat(f.path(), Is().EqualTo(path));

                // TODO: check content?
                AssertThat(f.exists(), Is().True());
                AssertThat(f.size(), Is().EqualTo(0u));
              });

            it("is marked persistent", [&path]() {
                file<int> f(path);
                AssertThat(f.is_persistent(), Is().True());
                AssertThat(f.is_temp(), Is().False());
              });

            it("is unchanged after marking it persistent once more", [&path]() {
                {
                  file<int> f(path);
                  f.make_persistent();

                  AssertThat(f.is_persistent(), Is().True());
                  AssertThat(f.is_temp(), Is().False());
                }
                AssertThat(std::filesystem::exists(path), Is().True());
              });

            it("cannot be 'moved'", [&path]() {
                std::string new_path = "./after-move-path.adiar";
                if (std::filesystem::exists(new_path)) {
                  // Clean up after prior test run
                  std::filesystem::remove(new_path);
                }

                file<int> f(path);
                std::string old_path = f.path();

                AssertThrows(std::runtime_error, f.move(new_path));
              });

            if (std::filesystem::exists(path)) {
              std::filesystem::remove(path);
            }
          });

        describe("::copy(const file&) [empty]", []() {
            it("can copy over a non-existing file", []() {
                file<int> f1;
                AssertThat(f1.exists(), Is().False());

                file<int> f2 = file<int>::copy(f1);
                AssertThat(f2.exists(), Is().False());

                AssertThat(f2.path(), Is().Not().EqualTo(f1.path()));
              });

            it("is unaffected by original file being 'touched'", []() {
                file<int> f1;
                file<int> f2 = file<int>::copy(f1);

                f1.touch();
                AssertThat(f2.exists(), Is().False());
              });

            it("can copy over a existing file", []() {
                file<int> f1;
                f1.touch();
                AssertThat(f1.exists(), Is().True());

                file<int> f2 = file<int>::copy(f1);
                AssertThat(f2.exists(), Is().True());
                AssertThat(f2.path(), Is().Not().EqualTo(f1.path()));
              });

            it("is temporary if original file is temporary", []() {
                file<int> f1;
                f1.touch();

                file<int> f2 = file<int>::copy(f1);
                AssertThat(f2.exists(), Is().True());
                AssertThat(f2.is_persistent(), Is().False());
                AssertThat(f2.is_temp(), Is().True());
              });

            it("is temporary if original file is persistent", []() {
                std::string path;
                {
                  file<int> f1;
                  f1.touch();
                  f1.make_persistent();

                  file<int> f2 = file<int>::copy(f1);
                  AssertThat(f2.exists(), Is().True());
                  AssertThat(f2.is_persistent(), Is().False());
                  AssertThat(f2.is_temp(), Is().True());

                  // clean up after f1
                  path = f1.path();
                }
                std::filesystem::remove(path);
              });
          });

        describe("file() [empty] + file_stream", []() {
            it("can attach to an empty file [constructor]", []() {
                file<int> f;
                file_stream<int> fs(f);
              });

            it("can attach to an empty file [member function]", []() {
                file<int> f;
                file_stream<int> fs;
                fs.attach(f);
              });

            it("remembers it was attached", []() {
              file<int> f;
              file_stream<int> fs(f);
              AssertThat(fs.attached(), Is().True());
            });

            it("cannot be pulled from", []() {
                file<int> f;
                file_stream<int> fs(f);

                AssertThat(fs.can_pull(), Is().False());
              });

            it("can be reset", []() {
                file<int> f;
                file_stream<int> fs(f);

                fs.reset();
                AssertThat(fs.attached(), Is().True());
                AssertThat(fs.can_pull(), Is().False());
              });

            it("can be detached again", []() {
              file<int> f;
              file_stream<int> fs(f);
              AssertThat(fs.attached(), Is().True());
              fs.detach();
              AssertThat(fs.attached(), Is().False());
            });
          });
      });
  });

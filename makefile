.PHONY: clean build test coverage

MAKE_FLAGS=-j $$(nproc)

# ============================================================================ #
#  BUILD
# ============================================================================ #
build:
	@mkdir -p build/ && cd build/ && cmake -D CMAKE_BUILD_TYPE=Release ..

# ============================================================================ #
#  clean
# ============================================================================ #
clean/files:
	@rm -rf *.tpie
	@rm -rf *.adiar*
	@rm -rf *.dot

clean: | clean/files
	@rm -r -f build/

# ============================================================================ #
#  TEST
# ============================================================================ #
O2_FLAGS = "-g -O2"

test:
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Debug -D CMAKE_C_FLAGS=$(O2_FLAGS) -D CMAKE_CXX_FLAGS=$(O2_FLAGS) ADIAR_STATS_EXTRA=ON ..
	@cd build/ && make $(MAKE_FLAGS) test_unit

	$(MAKE) clean/files

	@./build/test/test_unit --reporter=info --colorizer=light
	$(MAKE) clean/files

COV_C_FLAGS = "-g -O0 -Wall -fprofile-arcs -ftest-coverage"
COV_EXE_LINKER_FLAGS = "-fprofile-arcs -ftest-coverage"

coverage:
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Debug -D CMAKE_C_FLAGS=$(COV_C_FLAGS) -D CMAKE_CXX_FLAGS=$(COV_C_FLAGS) -D CMAKE_EXE_LINKER_FLAGS=$(COV_EXE_LINKER_FLAGS) ..
	@cd build/ && make $(MAKE_FLAGS) test_unit

	@lcov --directory build/src/adiar/ --zerocounters
	$(MAKE) clean/files

	@./build/test/test_unit
	$(MAKE) clean/files

  # create report
	@rm -rf test/report/
	@lcov --capture --directory build/ --output-file ./coverage.info
	@lcov --remove coverage.info --output-file coverage.info "/usr/*" "*/external/*" "./test/*"
  # print report to console
	@lcov --list coverage.info
  # print report to html file
	@genhtml coverage.info -o test/report/

# ============================================================================ #
#  MAIN for console debugging
# ============================================================================ #
M = 1024

main:
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Debug -D CMAKE_C_FLAGS=$(O2_FLAGS) -D CMAKE_CXX_FLAGS=$(O2_FLAGS) ADIAR_STATS_EXTRA=ON ..
	@cd build/ && make $(MAKE_FLAGS) adiar_main
	@echo "" && echo ""
	@./build/src/adiar_main ${M}

# ============================================================================ #
#  EXAMPLES
# ============================================================================ #

example/queens: N := 8
example/queens:
  # Build
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Release ..

	@cd build/ && make $(MAKE_FLAGS) queens

  # Run
	@echo ""
	./build/example/queens -N ${N} -M ${M}
	@echo ""


example/knights_tour/all: N := 5
example/knights_tour/all:
  # Build
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Release ..

	@cd build/ && make $(MAKE_FLAGS) knights_tour

  # Run
	@echo ""
	@./build/example/knights_tour -N ${N} -M ${M}
	@echo ""

example/knights_tour/closed: N := 6
example/knights_tour/closed:
  # Build
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Release ..

	@cd build/ && make $(MAKE_FLAGS) knights_tour

  # Run
	@echo ""
	@./build/example/knights_tour -N ${N} -M ${M} -c
	@echo ""

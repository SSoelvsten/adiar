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
clean-files:
	@rm -rf *.tpie
	@rm -rf *.adiar*
	@rm -rf *.dot

clean: | clean-files
	@rm -r -f build/

# ============================================================================ #
#  TEST
# ============================================================================ #
test:
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Debug -D ADIAR_TEST=ON ..
	@cd build/ && make $(MAKE_FLAGS) test_unit

	$(MAKE) clean-files

	@./build/test/test_unit --reporter=info --colorizer=light
	$(MAKE) clean-files

coverage:
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Debug -D CODE_COVERAGE=ON ..
	@cd build/ && make $(MAKE_FLAGS) test_unit

	@lcov --directory build/src/adiar/ --zerocounters
	$(MAKE) clean-files

	@./build/test/test_unit
	$(MAKE) clean-files

  # create report
	@rm -rf test/report/
	@lcov --capture --directory build/src/adiar/ --output-file ./coverage.info
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
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Debug ..
	@cd build/ && make $(MAKE_FLAGS) adiar_main
	@echo "" && echo ""
	@./build/src/adiar_main ${M}

# ============================================================================ #
#  EXAMPLES
# ============================================================================ #

example-queens: N := 8
example-queens:
  # Build
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Release ..

	@cd build/ && make $(MAKE_FLAGS) queens

  # Run
	@echo ""
	@./build/example/queens ${N} ${M}
	@echo ""

example-pigeonhole-principle: N := 10
example-pigeonhole-principle:
  # Build
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Release ..

	@cd build/ && make $(MAKE_FLAGS) pigeonhole_principle

  # Run
	@echo ""
	@./build/example/pigeonhole_principle ${N} ${M}
	@echo ""

example-tic-tac-toe: N := 20
example-tic-tac-toe:
  # Build
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Release ..

	@cd build/ && make $(MAKE_FLAGS) tic_tac_toe

  # Run
	@echo ""
	@./build/example/tic_tac_toe ${N} ${M}
	@echo ""

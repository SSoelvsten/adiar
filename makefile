.PHONY: clean build test

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
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Debug ..
	@cd build/ && make $(MAKE_FLAGS) test_unit

	$(MAKE) clean-files
	@./build/test/test_unit --reporter=info --colorizer=light
	$(MAKE) clean-files

# ============================================================================ #
#  DOT FILE output for visual debugging
# ============================================================================ #
F =

dot:
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Debug ..
	@cd build/ && make $(MAKE_FLAGS) adiar_dot
	@./build/src/adiar_dot ${F}

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

example-n-queens: N := 8
example-n-queens:
  # Build
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Release ..

	@cd build/ && make $(MAKE_FLAGS) n_queens

  # Run
	@echo ""
	@./build/example/n_queens ${N} ${M}
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

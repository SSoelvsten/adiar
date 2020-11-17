.PHONY: clean build test

MAKE_FLAGS=-j $$(nproc)

# ============================================================================ #
#  BUILD
# ============================================================================ #
build:
	@mkdir -p build/ && cd build/ && cmake ..

# ============================================================================ #
#  clean
# ============================================================================ #
clean-files:
	@rm -rf *.tpie
	@rm -rf *.coom*
	@rm -rf *.dot

clean: | clean-files
	@rm -r -f build/

# ============================================================================ #
#  TEST
# ============================================================================ #
test:
	@mkdir -p build/
	@cd build/ && cmake -D COOM_ASSERT=ON ..
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
	@cd build/ && cmake ..
	@cd build/ && make $(MAKE_FLAGS) coom_dot
	@./build/src/coom_dot ${F}

# ============================================================================ #
#  MAIN for console debugging
# ============================================================================ #
M = 1024

main:
	@mkdir -p build/
	@cd build/ && cmake -D COOM_ASSERT=ON ..
	@cd build/ && make $(MAKE_FLAGS) coom_main
	@echo "" && echo ""
	@./build/src/coom_main ${M}

# ============================================================================ #
#  EXAMPLES
# ============================================================================ #

example-n-queens: N := 8
example-n-queens:
  # Build
	@mkdir -p build/
	@cd build/ && cmake ..

	@cd build/ && make $(MAKE_FLAGS) n_queens

  # Run
	@echo ""
	@./build/example/n_queens ${N} ${M}
	@echo ""

example-pigeonhole-principle: N := 10
example-pigeonhole-principle:
  # Build
	@mkdir -p build/
	@cd build/ && cmake ..

	@cd build/ && make $(MAKE_FLAGS) pigeonhole_principle

  # Run
	@echo ""
	@./build/example/pigeonhole_principle ${N} ${M}
	@echo ""

example-tic-tac-toe: N := 20
example-tic-tac-toe:
  # Build
	@mkdir -p build/
	@cd build/ && cmake ..

	@cd build/ && make $(MAKE_FLAGS) tic_tac_toe

  # Run
	@echo ""
	@./build/example/tic_tac_toe ${N} ${M}
	@echo ""

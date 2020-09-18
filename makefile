.PHONY: clean build test

MAKE_FLAGS=-j $$(nproc)

# ============================================================================ #
#  BUILD
# ============================================================================ #
build:
	@mkdir -p build/ && cd build/ && cmake ..

build-test:
	@mkdir -p build/
	@cd build/ && cmake -DCOOM_DEBUG=OFF -DCOOM_ASSERT=ON ..
	@cd build/ && make $(MAKE_FLAGS) test_unit


# ============================================================================ #
#  CLEAN
# ============================================================================ #
clean:
	@rm -r -f build/

# ============================================================================ #
#  TEST
# ============================================================================ #
test: | build-test
	@rm -rf *.tpie
	@./build/test/test_unit --reporter=info --colorizer=light
	@rm -rf *.tpie

# ============================================================================ #
#  DOT FILE output for visual debugging
# ============================================================================ #
F =

dot:
	@mkdir -p build/
	@cd build/ && cmake -DCOOM_DEBUG=OFF -DCOOM_ASSERT=OFF ..
	@cd build/ && make $(MAKE_FLAGS) coom_dot
	@./build/src/coom_dot ${F}

# ============================================================================ #
#  MAIN for console debugging
# ============================================================================ #
main:
	@mkdir -p build/
	@cd build/ && cmake -DCOOM_DEBUG=ON -DCOOM_ASSERT=ON ..
	@cd build/ && make $(MAKE_FLAGS) coom_main
	@rm -rf *.tpie
	@echo "" && echo ""
	@./build/src/coom_main
	@rm -rf *.tpie

# ============================================================================ #
#  EXAMPLES
# ============================================================================ #
M = 1

example-n-queens: N := 8
example-n-queens:
  # Build
	@mkdir -p build/
	@cd build/ && cmake -DCOOM_DEBUG=OFF -DCOOM_ASSERT=OFF ..

	@cd build/ && make $(MAKE_FLAGS) n_queens

  # Run
	@echo ""
	@./build/example/n_queens ${N} ${M}
	@echo ""

example-tic-tac-toe: N := 20
example-tic-tac-toe:
  # Build
	@mkdir -p build/
	@cd build/ && cmake -DCOOM_DEBUG=OFF -DCOOM_ASSERT=OFF ..

	@cd build/ && make $(MAKE_FLAGS) tic_tac_toe

  # Run
	@echo ""
	@./build/example/tic_tac_toe ${N} ${M}
	@echo ""

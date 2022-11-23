.PHONY: build clean coverage docs test test/*

MAKE_FLAGS=-j $$(nproc)

# ============================================================================ #
#  BUILD
# ============================================================================ #
BUILD_TYPE = "Release"

build:
	$(MAKE) build/static

build/static:
	@mkdir -p build/ && cd build/ && cmake -D CMAKE_BUILD_TYPE=$(BUILD_TYPE) ..
	@cd build/ && make adiar

build/shared:
	@echo "Not supportd (See Issue #200)"

# ============================================================================ #
#  CLEAN
# ============================================================================ #
clean/files:
	@rm -rf *.tpie
	@rm -rf *.adiar*
	@rm -rf *.dot

clean: | clean/files
	@rm -r -f build/

# ============================================================================ #
#  UNIT TESTING
# ============================================================================ #
O2_FLAGS = "-g -O2"

TEST_NAME = test-adiar

test: TEST_FOLDER = test
test: TEST_NAME = adiar
test:
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Debug \
                      -D CMAKE_C_FLAGS=$(O2_FLAGS) \
                      -D CMAKE_CXX_FLAGS=$(O2_FLAGS) \
                      -D ADIAR_STATS=ON \
                ..

	@cd build/ && make $(MAKE_FLAGS) $(subst /,-,$(TEST_FOLDER))-$(TEST_NAME)

	$(MAKE) clean/files
	@./build/$(TEST_FOLDER)/$(subst /,-,$(TEST_FOLDER))-$(TEST_NAME) \
      --reporter=info --colorizer=light

#	@./build/test/$(TEST_NAME) --reporter=info --colorizer=light
	$(MAKE) clean/file

test/all:
	make $(MAKE_FLAGS) test

test/adiar:
	make $(MAKE_FLAGS) test

test/adiar/assignment:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar TEST_NAME=assignment

test/adiar/bool_op:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar TEST_NAME=bool_op

test/adiar/builder:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar TEST_NAME=builder

test/adiar/domain:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar TEST_NAME=domain

test/adiar/bdd/apply:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=apply

test/adiar/bdd/assignment:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=assignment

test/adiar/bdd/bdd:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=bdd

test/adiar/bdd/build:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=build

test/adiar/bdd/count:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=count

test/adiar/bdd/evaluate:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=evaluate

test/adiar/bdd/if_then_else:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=if_then_else

test/adiar/bdd/negate:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=negate

test/adiar/bdd/quantify:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=quantify

test/adiar/bdd/restrict:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/bdd TEST_NAME=restrict

test/adiar/internal/dot:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal TEST_NAME=dot

test/adiar/internal/util:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal TEST_NAME=util

test/adiar/internal/algorithms/convert:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/algorithms TEST_NAME=convert

test/adiar/internal/algorithms/isomorphism:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/algorithms TEST_NAME=isomorphism

test/adiar/internal/algorithms/reduce:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/algorithms TEST_NAME=reduce

test/adiar/internal/data_structures/levelized_priority_queue:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_structures TEST_NAME=levelized_priority_queue

test/adiar/internal/data_types/arc:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_types TEST_NAME=arc

test/adiar/internal/data_types/convert:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_types TEST_NAME=convert

test/adiar/internal/data_types/node:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_types TEST_NAME=node

test/adiar/internal/data_types/ptr:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_types TEST_NAME=ptr

test/adiar/internal/data_types/tuple:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_types TEST_NAME=tuple

test/adiar/internal/data_types/uid:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/data_types TEST_NAME=uid

test/adiar/internal/io/file:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/internal/io TEST_NAME=file

test/adiar/zdd/binop:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=binop

test/adiar/zdd/build:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=build

test/adiar/zdd/change:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=change

test/adiar/zdd/complement:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=complement

test/adiar/zdd/contains:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=contains

test/adiar/zdd/count:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=count

test/adiar/zdd/elem:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=elem

test/adiar/zdd/expand:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=expand

test/adiar/zdd/pred:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=pred

test/adiar/zdd/project:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=project

test/adiar/zdd/subset:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=subset

test/adiar/zdd/zdd:
	make $(MAKE_FLAGS) test TEST_FOLDER=test/adiar/zdd TEST_NAME=zdd

# ============================================================================ #
#  LCOV COVERAGE REPORT
# ============================================================================ #
COV_C_FLAGS = "-g -O0 -Wall -fprofile-arcs -ftest-coverage"
COV_EXE_LINKER_FLAGS = "-fprofile-arcs -ftest-coverage"

coverage:
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Debug \
                      -D CMAKE_C_FLAGS=$(COV_C_FLAGS) \
                      -D CMAKE_CXX_FLAGS=$(COV_C_FLAGS) \
                      -D CMAKE_EXE_LINKER_FLAGS=$(COV_EXE_LINKER_FLAGS) \
                      -D ADIAR_STATS=ON \
                ..
	@cd build/ && make $(MAKE_FLAGS) test-adiar

	@lcov --directory build/src/adiar/ --zerocounters
	$(MAKE) clean/files

	@./build/test/test-adiar
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
#  DOCUMENTATION
# ============================================================================ #
docs:
	@mkdir -p build/
	@cd build/ && cmake ..

	@cd build/ && $(MAKE) adiar_docs

# ============================================================================ #
#  MAIN program for small tests
# ============================================================================ #
M = 1024

main:
	@mkdir -p build/
	@cd build/ && cmake -D CMAKE_BUILD_TYPE=Debug \
                      -D CMAKE_C_FLAGS=$(O2_FLAGS) \
                      -D CMAKE_CXX_FLAGS=$(O2_FLAGS) \
                      -D ADIAR_STATS=ON \
                ..
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

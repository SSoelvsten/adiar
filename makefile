.PHONY: clean build test

# ============================================================================ #
#  BUILD
# ============================================================================ #
build-cmake:
	@mkdir -p build/ && cd build/ && cmake ..

build-test: | build-cmake
	@cd build/ && make test_unit

build: | build-cmake build-test

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
#  SETUP DEPENDENCIES
#
#  TODO: Remove on publication...
# ============================================================================ #
setup: | setup-submodules setup-c

setup-c:
	sudo apt install g++

	sudo apt install cmake
	sudo apt install libboost-all-dev
	sudo apt install aptitude

setup-submodules:
	git submodule update --init --recursive

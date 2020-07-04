.PHONY: clean build test

# ============================================================================ #
#  BUILD
# ============================================================================ #
build:
	@mkdir -p build/ && cd build/ && cmake ..

build-test:
	@mkdir -p build/
	@cd build/ && cmake -DCOOM_DEBUG=OFF -DCOOM_ASSERT=ON ..
	@cd build/ && make test_unit

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

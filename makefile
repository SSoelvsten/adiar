test: | compile-test run-test clean-test

compile-test:
	@g++ -Itest/ -std=c++11 -o run_test.out test/test.cpp

run-test: | compile-test
	@./run_test.out --reporter=info --colorizer=light

clean-test:
	@rm run_test.out


TEST_CMD = ./producer one two three

all: producer

producer:
	@echo
	@echo "======================================================[ Building 'producer' ]==="
	g++ -std=c++0x -Wall -Werror -o producer producer.cpp

test: producer
	@echo
	@echo "=============================================================[ Running test ]==="
	@echo $(TEST_CMD)
	@echo
	@echo "===[ Results ]:"
	@$(TEST_CMD) 2>&1 | sed 's/^/   /'

clean:
	@echo
	@echo "=================================================================[ Cleaning ]==="
	rm -f producer

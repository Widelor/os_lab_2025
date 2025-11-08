CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic
LDFLAGS = -lpthread

all: mutex factorial deadlock

# Программа 1: Пример с мьютексом
mutex: mutex.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Программа 2: Параллельное вычисление факториала
factorial: factorial.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Программа 3: Демонстрация deadlock
deadlock: deadlock.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test-mutex: mutex
	@echo "=== Testing mutex program ==="
	@echo "Running without synchronization:"
	@./mutex || true
	@echo
	@echo "Note: Uncomment mutex locks in code to see synchronized version"

test-factorial: factorial
	@echo "=== Testing factorial program ==="
	@./factorial -k 5 --pnum=2 --mod=100
	@./factorial -k 10 --pnum=4 --mod=1000

test-deadlock: deadlock
	@echo "=== Testing deadlock program ==="
	@echo "This will deadlock - press Ctrl+C after a few seconds to stop"
	@-timeout 5s ./deadlock || true

test: test-mutex test-factorial test-deadlock

clean:
	rm -f mutex factorial deadlock

help:
	@echo "Available targets:"
	@echo "  all        - Build all programs (default)"
	@echo "  mutex      - Build mutex example"
	@echo "  factorial  - Build factorial calculator"
	@echo "  deadlock   - Build deadlock demonstration"
	@echo "  test       - Run all tests"
	@echo "  test-*     - Run specific test"
	@echo "  clean      - Remove compiled programs"
	@echo "  help       - Show this help"

.PHONY: all test test-mutex test-factorial test-deadlock clean help
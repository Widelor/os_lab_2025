# Makefile для сборки parallel_min_max и process_memory

# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -pthread

# Цели по умолчанию
all: parallel_min_max process_memory parallel_sum

# Программа 1: parallel_min_max
parallel_min_max: parallel_min_max.o find_min_max.o utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

parallel_min_max.o: parallel_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) -c parallel_min_max.c

find_min_max.o: find_min_max.c find_min_max.h
	$(CC) $(CFLAGS) -c find_min_max.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

# Программа 3: process_memory
process_memory: process_memory.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

process_memory.o: process_memory.c
	$(CC) $(CFLAGS) -c process_memory.c

# parallel_sum
parallel_sum: parallel_sum.o sum.o utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

parallel_sum.o: parallel_sum.c sum.h utils.h
	$(CC) $(CFLAGS) -c parallel_sum.c

sum.o: sum.c sum.h
	$(CC) $(CFLAGS) -c sum.c

test: parallel_sum
	@echo "=== Тестирование parallel_sum ==="
	./parallel_sum --threads_num 4 --array_size 1000 --seed 123

# Тестовые цели
test_parallel: parallel_min_max
	@echo "=== Тестирование parallel_min_max ==="
	./parallel_min_max --seed 123 --array_size 100 --pnum 4
	@echo ""
	./parallel_min_max --seed 123 --array_size 100 --pnum 4 --by_files
	@echo ""
	./parallel_min_max --seed 123 --array_size 100 --pnum 4 --timeout 5

test_memory: process_memory
	@echo "=== Тестирование process_memory ==="
	./process_memory

# Очистка
clean:
	rm -f *.o
	rm -f parallel_min_max process_memory
	rm -f min_max_*.txt  # Удаляем временные файлы, созданные parallel_min_max
	rm -f *.o parallel_sum

# Установка (если нужно)
install: all
	@echo "Программы собраны и готовы к использованию"

# Справка
help:
	@echo "Доступные цели:"
	@echo "  all           - собрать все программы (по умолчанию)"
	@echo "  parallel_min_max - собрать только parallel_min_max"
	@echo "  process_memory   - собрать только process_memory"
	@echo "  test_parallel - протестировать parallel_min_max"
	@echo "  test_memory   - протестировать process_memory"
	@echo "  clean         - удалить все скомпилированные файлы"
	@echo "  install       - собрать все программы"
	@echo "  help          - показать эту справку"

# Объявляем фиктивные цели
.PHONY: all clean install help test test_parallel test_memory
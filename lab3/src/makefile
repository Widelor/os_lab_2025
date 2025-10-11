CC=gcc
CFLAGS=-I.

# Default target - build all programs
all: sequential_min_max parallel_min_max exec_sequential

# Sequential program
sequential_min_max: utils.o find_min_max.o
	$(CC) -o sequential_min_max sequential_min_max.c find_min_max.o utils.o $(CFLAGS)

# Parallel program  
parallel_min_max: utils.o find_min_max.o
	$(CC) -o parallel_min_max parallel_min_max.c find_min_max.o utils.o $(CFLAGS)

# Exec program that runs sequential_min_max
exec_sequential: sequential_min_max
	$(CC) -o exec_sequential exec_sequential.c $(CFLAGS)

# Object files
utils.o: utils.c utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o: find_min_max.c find_min_max.h utils.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

# Clean target
clean:
	rm -f utils.o find_min_max.o sequential_min_max parallel_min_max exec_sequential

# Phony targets (not actual files)
.PHONY: all clean
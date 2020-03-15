# What compiler to use
CC=gcc

# Paths to source and lib
SRC_DIR=src
DEPS=$(wildcard $(SRC_DIR)/*.h)
SRC=$(wildcard $(SRC_DIR)/*.c)

# Build flags to use
CFLAGS=-I$(SRC_DIR)

# Path where compiled files should be build to
BUILD_DIR=build

#
main:$(SRC)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/main.out $^

CFLAGS = -Wall -g -Iinclude
CC = gcc

# Dossiers sources
SRC_DIR = src
TEST_DIR = test

# Sources
SRC_FILES = $(SRC_DIR)/dma.c $(SRC_DIR)/dma_sim.c
TEST_FILES = $(TEST_DIR)/main1.c $(TEST_DIR)/test_helpers.c $(TEST_DIR)/verif.c

# Objets
SRC_OBJ = $(SRC_FILES:.c=.o)
TEST_OBJ = $(TEST_FILES:.c=.o)

.PHONY: all clean

all: test1

test1: $(SRC_OBJ) $(TEST_OBJ)
	$(CC) -o $@ $(CFLAGS) $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_OBJ) $(TEST_OBJ) *~ test1
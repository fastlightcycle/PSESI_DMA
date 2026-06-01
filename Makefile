CFLAGS = -Wall -g -Iinclude
CC = gcc

# Dossiers sources
SRC_DIR = src
TEST_DIR = test

# Sources
SRC_FILES = $(SRC_DIR)/dma.c $(SRC_DIR)/dma_sim.c

# Objets
SRC_OBJ = $(SRC_FILES:.c=.o)

TEST_MAINS = main1_1 main2
TEST_EXES = $(addprefix $(TEST_DIR)/,$(TEST_MAINS))

.PHONY: all clean

all: $(TEST_EXES)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_DIR)/%: $(TEST_DIR)/%.c $(SRC_OBJ)
	$(CC) $(CFLAGS) -o $@ $< $(SRC_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_OBJ) $(TEST_EXES) $(TEST_DIR)/*.o *~

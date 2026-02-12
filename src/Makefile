CFLAGS = -Wall -g 
CC = gcc

PROGRAMS = sim_dma # tests

.PHONY:	all clean #Commande 'make' rettrouve donc 'all' en premier

all: $(PROGRAMS)

sim_dma: dma.o dma_sim.o main.o
	$(CC) -o $@ $(CFLAGS) $^

# tests: main_thread.o dma.o dma_sim.o
# 	$(CC) -o $@ $(CFLAGS) $^

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f *.o *~ sim_dma tests

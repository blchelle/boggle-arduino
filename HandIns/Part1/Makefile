#Names: Brock Chelle, Benjamin Wagg
#IDs: 1533398, 1531566
#CCID: bchelle, bwagg
#Course: CMPUT 275
#Term: Winter 2019
#Final Project: Boggle Solver (Part 1)

CC = g++
OBJS = solver.o
LFLAGS = -static
CFLAGS = -c -std=c++11
PROGRAM = solver

$(PROGRAM): $(OBJS)
	@$(CC) $(OBJS) -o $(PROGRAM) $(LFLAGS)

solver.o: solver.cpp trie.h digraph.h
	@$(CC) solver.cpp -o solver.o $(CFLAGS)

clean: $(OBJS) $(PROGRAM) 
	@rm -f $(OBJS)
	@rm -f $(PROGRAM)

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

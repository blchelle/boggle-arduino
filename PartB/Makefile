CC = g++
OBJS = solver.o
LFLAGS = -static
CFLAGS = -c -std=c++11
PROGRAM = solver

$(PROGRAM): $(OBJS)
	@$(CC) $(OBJS) -o $(PROGRAM) $(LFLAGS)

solver.o: solver.cpp trie.h digraph.h
	@$(CC) solver.cpp -o solver.o $(CFLAGS)

# Not for running the main file
trie: createTrie.cpp trie.h
	@$(CC) createTrie.cpp -o trie $(CFLAGS)

# Not for running the main file
cut: trim-dictionary.cpp
	@$(CC) trim-dictionary.cpp -o cut $(CFLAGS)

clean: $(OBJS) $(PROGRAM) 
	@rm -f $(OBJS)
	@rm -f $(PROGRAM)

CC = g++
OBJS = server.o serial.o
LFLAGS = -static
CFLAGS = -c -std=c++11
PROGRAM = server

both: $(PROGRAM)
	cd ../Client/ && make upload
	cd ../Server/ && make $(PROGRAM) && ./$(PROGRAM)

$(PROGRAM): $(OBJS)
	@$(CC) $(OBJS) -o $(PROGRAM) $(LFLAGS)

server.o: server.cpp trie.h digraph.h
	@$(CC) server.cpp -o server.o $(CFLAGS)

serial.o: serialport.cpp serialport.h
	@$(CC) serialport.cpp -o serial.o $(CFLAGS)

# Not for running the main file
trie: createTrie.cpp trie.h
	@$(CC) createTrie.cpp -o trie $(CFLAGS)

# Not for running the main file
cut: trim-dictionary.cpp
	@$(CC) trim-dictionary.cpp -o cut $(CFLAGS)

clean: $(OBJS) $(PROGRAM) 
	@rm -f $(OBJS)
	@rm -f $(PROGRAM)

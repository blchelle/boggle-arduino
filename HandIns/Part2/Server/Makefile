#Names: Brock Chelle, Benjamin Wagg
#IDs: 1533398, 1531566
#CCID: bchelle, bwagg
#Course: CMPUT 275
#Term: Winter 2019
#Final Project: Boggle Solver (Part 2)

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

clean: $(OBJS) $(PROGRAM) 
	@rm -f $(OBJS)
	@rm -f $(PROGRAM)

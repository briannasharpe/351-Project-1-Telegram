all: sender recv

sender.o: sender.cpp
	g++ -c sender.cpp

recv.o: recv.cpp
	g++ -c recv.cpp

sender: sender.o
	g++ sender.o -o sender

recv: recv.o
	g++ recv.o -o recv
#/bin/bash

rm *.o

echo "Compile .cpp files"
g++ -c sender.cpp
g++ -c recv.cpp

echo "Link .o files"
g++ -o sender.o recv.o

echo "Run executable"
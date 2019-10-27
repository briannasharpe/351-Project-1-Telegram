
GXX49_VERSION := $(shell g++-4.9 --version 2>/dev/null)

ifdef GXX49_VERSION
	CXX_COMMAND := g++-4.9
else
	CXX_COMMAND := g++
endif

CXX = ${CXX_COMMAND} -std=c++11 -Wall


all: recv sender

headers: msg.h
	
recv: headers recv.cpp
	${CXX} recv.cpp -o receiver

sender: headers sender.cpp
	${CXX} sender.cpp -o sender

clean:
	rm -f sender recv


GXX49_VERSION := $(shell g++-4.9 --version 2>/dev/null)

ifdef GXX49_VERSION
	CXX_COMMAND := g++-4.9
else
	CXX_COMMAND := g++
endif

CXX = ${CXX_COMMAND} -std=c++11 -Wall

run_test: signaldemo
	./signaldemo

headers: msg.h

sender: headers sender.cpp
	${CXX} sender.cpp -o sender
	
recv: headers recv.cpp
	${CXX} recv.cpp -o recv

signaldemo: headers signaldemo.cpp
	${CXX} signaldemo.cpp -o signaldemo

clean:
	rm -f sender recv signaldemo

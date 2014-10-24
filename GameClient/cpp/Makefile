all:
	g++ -o emps emps.cpp -L./ -pthread -lzmq -ljson_linux-gcc-4.6_libmt -std=gnu++0x
clean:
	rm emps
#CC=g++
#CFLAGS=-c -g -Wall
#LDFLAGS=-L./ -pthread -lzmq -ljson_linux-gcc-4.6_libmt
#SOURCES=emps.cpp
#OBJECTS=$(SOURCES:.cpp=.o)
#EXECUTABLE=emps
#all: $(SOURCES) $(EXECUTABLE)
#$(EXECUTABLE): $(OBJECTS)
	#$(CC) $(LDFLAGS) $(OBJECTS) -o $@
#
#.cpp.o:
	#$(CC) $(CFLAGS) $< -o $@
#
#clean:
	#rm emps

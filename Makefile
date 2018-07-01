CFLAGS=-O3 -std=c++11

simulator: ALU.o CPU.o Memory.o Stats.o Simulator.o
	g++ $(CFLAGS) ALU.o CPU.o Memory.o Stats.o Simulator.o -o simulator

ALU.o: Debug.h ALU.h ALU.cpp
	g++ $(CFLAGS) -c ALU.cpp

CPU.o: Debug.h ALU.h Memory.h Stats.h CPU.h CPU.cpp
	g++ $(CFLAGS) -c CPU.cpp

Memory.o: Debug.h Memory.h Memory.cpp
	g++ $(CFLAGS) -c Memory.cpp

Stats.o: Debug.h Stats.h Stats.cpp
	g++ $(CFLAGS) -c Stats.cpp

Simulator.o: Debug.h CPU.h Memory.h Stats.h Simulator.cpp
	g++ $(CFLAGS) -c Simulator.cpp

.PHONY: clean
clean:
	rm -f ALU.o CPU.o Memory.o Stats.o Simulator.o simulator

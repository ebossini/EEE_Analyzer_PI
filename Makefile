
CC = g++
CFLAGS = -c -Wall -malign-double
EXECUTABLE = EEE_Analyzer_PI_V0


all: EEE_Analyzer_PI

EEE_Analyzer_PI: E3Event.o E3Gps.o E3Run.o
	$(CC) E3Event.o E3Gps.o E3Run.o EEE_Analyzer_PI.cpp  -o $(EXECUTABLE)	

E3Event.o: E3Event.cpp
	$(CC) $(CFLAGS) E3Event.cpp

E3Gps.o: E3Gps.cpp
	$(CC) $(CFLAGS) E3Gps.cpp

E3Run.o: E3Run.cpp
	$(CC) $(CFLAGS) E3Run.cpp

clean:
	rm -rf *o *~

cleanall: clean
	rm -rf $(EXECUTABLE)

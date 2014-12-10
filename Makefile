
CC = g++
CFLAGS = -c -Wall -malign-double
EXECUTABLE = EEE_Analyzer_PI_V2


all: EEE_Analyzer_PI

EEE_Analyzer_PI: E3Cluster.o E3Event.o E3Fitter.o E3Gps.o E3Hit.o E3Point.o E3RawData.o E3RecoEvent.o E3Run.o E3StripData.o E3Track.o E3Vector.o stdafx.o
	$(CC) E3Cluster.o E3Event.o E3Fitter.o E3Gps.o E3Hit.o E3Point.o E3RawData.o E3RecoEvent.o E3Run.o E3StripData.o E3Track.o E3Vector.o stdafx.o EEE_Analyzer_PI.cpp  -o $(EXECUTABLE)

E3Cluster.o: E3Cluster.cpp
	$(CC) $(CFLAGS) E3Cluster.cpp

E3Event.o: E3Event.cpp
	$(CC) $(CFLAGS) E3Event.cpp

E3Fitter.o: E3Fitter.cpp
	$(CC) $(CFLAGS) E3Fitter.cpp

E3Gps.o: E3Gps.cpp
	$(CC) $(CFLAGS) E3Gps.cpp

E3Hit.o: E3Hit.cpp
	$(CC) $(CFLAGS) E3Hit.cpp

E3Point.o: E3Point.cpp
	$(CC) $(CFLAGS) E3Point.cpp

E3RawData.o: E3RawData.cpp
	$(CC) $(CFLAGS) E3RawData.cpp

E3RecoEvent.o: E3RecoEvent.cpp
	$(CC) $(CFLAGS) E3RecoEvent.cpp

E3Run.o: E3Run.cpp
	$(CC) $(CFLAGS) E3Run.cpp

E3StripData.o: E3StripData.cpp
	$(CC) $(CFLAGS) E3StripData.cpp

E3Track.o: E3Track.cpp
	$(CC) $(CFLAGS) E3Track.cpp

E3Vector.o: E3Vector.cpp
	$(CC) $(CFLAGS) E3Vector.cpp

stdafx.o: stdafx.cpp
	$(CC) $(CFLAGS) stdafx.cpp

clean:
	rm -rf *o *~

cleanall: clean
	rm -rf $(EXECUTABLE)

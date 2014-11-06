all: EEE_Analyzer_PI

EEE_Analyzer_PI: E3Event.o E3Gps.o E3Run.o

E3Event.o: E3Event.cpp
	g++ -c E3Event.cpp

E3Gps.o: E3Gps.cpp
	g++ -c E3Gps.cpp

E3Run.o: E3Run.cpp
	g++ -c E3Run.cpp

clean:
	rm -rf *o *~

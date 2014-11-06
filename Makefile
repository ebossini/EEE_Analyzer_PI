all: EEE_Analyzer_PI

EEE_Analyzer_PI: E3Event.o E3Gps.o E3Run.o
	g++ E3Event.o E3Gps.o E3Run.o EEE_Analyzer_PI.cpp -o EEE_Analyzer_PI_V0

E3Event.o: E3Event.cpp
	g++ -c E3Event.cpp

E3Gps.o: E3Gps.cpp
	g++ -c E3Gps.cpp

E3Run.o: E3Run.cpp
	g++ -c E3Run.cpp

clean:
	rm -rf *o *~

cleanall: clean
	rm -rf EEE_Analyzer_PI_V0

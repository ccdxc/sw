teststub:
	gcc -fpic -c pal.cc -o pal.o -std=c++11 -lstdc++
	gcc -shared pal.o -o libpal.so
	gcc planner.cc -o planner -std=c++11 -lstdc++ -lpal -L.

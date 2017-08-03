INCS = -I../../saratk/nic/model_sim/include/

all:
	make mock_model

mock_model:
	g++ $(INCS) -g -o ./mockmodel.o -c -g --std=c++11 -c mockmodel.cc
	g++ -o mockmodel mockmodel.o -lzmq

clean:
	rm -f *.o
	rm -f mockmodel

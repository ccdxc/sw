TOP_DIR = ./../..
CXX = g++
SWG = swig
SWGFLAGS = -python -c++
CPPFLAGS = -g -c -pthread -std=c++11 -fPIC
LNKFLAGS = -Wl,-rpath,./obj -Wl,-rpath,./model_sim/build -Wl,-rpath,./model_sim/libs
LNKFLAGS += -Wl,-rpath,$(TOP_DIR)/obj -Wl,-rpath,$(TOP_DIR)/model_sim/build -Wl,-rpath,$(TOP_DIR)/model_sim/libs -shared
BLDFLAGS = -Wl,-rpath,./obj -Wl,-rpath,./model_sim/build -Wl,-rpath,./model_sim/libs
BLDFLAGS += -Wl,-rpath,$(TOP_DIR)/obj -Wl,-rpath,$(TOP_DIR)/model_sim/build -Wl,-rpath,$(TOP_DIR)/model_sim/libs
INC_DIRS = -I./../include
INC_DIRS += -I$(TOP_DIR)/model_sim/include
INC_DIRS += -I/usr/include/python2.7
LIB_DIRS = -L$(TOP_DIR)/obj -L$(TOP_DIR)/model_sim/build -L$(TOP_DIR)/model_sim/libs
LIBS = -lpython2.7 -lmodelclient -lzmq

all:
	$(SWG) $(SWGFLAGS) $(INC_DIRS) -o model_wrap.c model.i
	$(CXX) $(CPPFLAGS) $(INC_DIRS) -o model_wrap.o model_wrap.c
	$(CXX) $(LNKFLAGS) -o _model.so model_wrap.o $(LIB_DIRS) $(LIBS)

clean:
	rm *.o *.so *.pyc model.py model_wrap.c

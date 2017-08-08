# This Makefile is auto-generated. Changes will be overwritten!
CXX = g++
SWG = swig
SWGFLAGS = -c++ -python
CPPFLAGS = -g -c -pthread -std=c++11 -fPIC
LNKFLAGS = -Wl,-rpath,../../../../../../obj -shared
INC_DIRS = -I.
INC_DIRS += -I../include
INC_DIRS += -I../../../../../../hal/pd/
INC_DIRS += -I/usr/local/include -I/usr/include/boost/
INC_DIRS += -I/usr/include/python2.7 -I/usr/include/x86_64-linux-gnu/python2.7
LIB_DIRS = -L/usr/local/lib -L../../../../../../obj -L/usr/lib/python2.7/dist-packages -L.
LIBS = -lpython2.7 -lcapri -lcaprimrl -ltrace

all:
	$(SWG) $(SWGFLAGS) -o p4pd_wrap.c p4pd.i
	$(CXX) $(CPPFLAGS) $(INC_DIRS) -o p4pd.o ../src/p4pd.c
	$(CXX) $(CPPFLAGS) $(INC_DIRS) -o stub.o stub.cc
	$(CXX) $(CPPFLAGS) $(INC_DIRS) -o p4pd_wrap.o p4pd_wrap.c
	$(CXX) $(LNKFLAGS) -o _p4pd.so *.o $(LIB_DIRS) $(LIBS)
clean:
	rm *.o *.so p4pd.py p4pd_wrap.c

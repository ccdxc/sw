
TOPDIR = ../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

all:
	make model
	make sample
	make cli

coverage:
	g++ -O3 --coverage -o ./model_server.o -DCOVERAGE -c -g -I/home/asic/tools/src/0.25/include -I/usr/include/python2.7 -std=c++11 -I/home/asic/tools/src/0.25/include -I/usr/include/python2.7 -I../include ../src/model_server.cc
	g++ -O3 -o cap_model --coverage -DCOVERAGE -Wl,-rpath,/usr/local/lib64 -Wl,-rpath,/usr/local/lib64 /home/asic/tools/src/0.25/x86_64/lib64/libsknobs.so model_server.o -L/home/asic/tools/src/0.25/x86_64/lib64 -L../libs -lpthread -ldl -lutil -lgmp -lgmpxx -lpython2.7 -lmodel_cc -lcommon_cc -lJudy -lsknobs -ltop_csr_compile -lcapsim -lisa -lasmsym -lzmq -lcapisa -lmpuobj -lcrypto -lgcov -lmpuobj
	g++ -O3 --coverage -o ./libmodelclient.so -DCOVERAGE -c -g -I/home/asic/tools/src/0.25/include -I/usr/include/python2.7 -std=c++11 -I/home/asic/tools/src/0.25/include -I/usr/include/python2.7 -I../include ../src/lib_model_client.cc -lzmq -fPIC

model:
	g++ -O3 -o ./model_server.o -c -g -I/home/asic/tools/src/0.25/include -I/usr/include/python2.7 -std=c++11 -I/home/asic/tools/src/0.25/include -I/usr/include/python2.7 -I../include ../src/model_server.cc
	g++ -O3 -o cap_model -Wl,-rpath,/usr/local/lib64 -Wl,-rpath,/usr/local/lib64 /home/asic/tools/src/0.25/x86_64/lib64/libsknobs.so model_server.o -L/home/asic/tools/src/0.25/x86_64/lib64 -L../libs -lpthread -ldl -lutil -lgmp -lgmpxx -lpython2.7 -lmodel_cc -lcommon_cc -lJudy -lsknobs -ltop_csr_compile -lcapsim -lisa -lasmsym -lzmq -lcapisa -lcrypto -lmpuobj
	g++ -O3 -o ./libmodelclient.so -c -g -I/home/asic/tools/src/0.25/include -I/usr/include/python2.7 -std=c++11 -I/home/asic/tools/src/0.25/include -I/usr/include/python2.7 -I../include ../src/lib_model_client.cc -lzmq -fPIC

sample:
	g++ -g -o sample_client -std=c++11 -L./ -I../include -lmodelclient -lzmq ../examples/sample_client.c
	g++ -g -o sample_client_tcp -std=c++11 -L./ -I../include -lmodelclient -lzmq ../examples/sample_client_tcp.c

cli:
	g++ -g -o model_cli -L./ -I../include -lmodelclient -lzmq ../examples/model_cli.c

clean:
	rm -f *.o
	rm -f *.so
	rm -f sample_client
	rm -f cap_model

teststub:
	rm -rf obj
	mkdir obj
	gcc -fpic -c lib/malloc_stub.cc -o obj/malloc_stub.o
	gcc -shared obj/malloc_stub.o -o obj/libmallocstub.so
	gcc -fpic -c lib/mom.cc -o obj/mom.o
	gcc -shared obj/mom.o -o obj/libmom.so -std=c++11
	gcc -Lobj/ -lmallocstub -lmom mom_test.cc -o obj/test_mom

clean:
	rm -rf obj

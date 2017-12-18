OBJECTS = ddd.o

default: ddd

%.o: %.c
	gcc -c $< -o $@

program: $(OBJECTS)
	gcc $(OBJECTS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f ddd

OBJECTS = ddd.o

default: ddd

%.o: %.c
	gcc -I ../../../../../common -c $< -o $@

program: $(OBJECTS)
	gcc $(OBJECTS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f ddd

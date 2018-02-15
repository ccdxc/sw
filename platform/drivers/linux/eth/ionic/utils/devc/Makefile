OBJECTS = devc.o

default: devc

%.o: %.c
	gcc -c $< -o $@

program: $(OBJECTS)
	gcc $(OBJECTS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f devc

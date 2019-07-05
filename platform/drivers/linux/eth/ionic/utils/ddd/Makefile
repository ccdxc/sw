OBJECTS = ddd.o

default: ddd

%.o: %.c
	$(CC) -I ../../../../../common -c $< -o $@

program: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f ddd

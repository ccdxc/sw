OBJECTS = devc.o
INC = -I../../../../../common

default: devc

%.o: %.c
	gcc $(INC) -c $< -o $@

program: $(OBJECTS)
	gcc $(OBJECTS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f devc

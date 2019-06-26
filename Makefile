sources = $(wildcard *.c)
objects = $(addsuffix .o, $(basename $(sources)))
flags = -g -W -Wall -std=c11 -DDEBUG=0 -DTEST=0
target = network

$(target) : $(objects)
	gcc -o $(target) $(objects) -lm

%.o : %.c
	gcc -c $(flags) $< -o $@

clean :
	rm $(target) $(objects)

.DEFAULT_GOAL := cmp
CFLAGS=-std=c11 -g
LIBS=$(wildcard ./lib/*.c)
OBJS=$(LIBS:.c=.o)

cmp: $(LIBS)
	cc -o cmp $(CFLAGS) ./bin/cmp.c $(LIBS)
print: $(LIBS)
	cc -o print $(CFLAGS) ./bin/print.c $(LIBS)
	./print "$(ARGS)"
$(OBJS): src/cmp.h
test: cmp
	./test.sh

run: cmp
	./cmp "$(ARGS)" > a.s
	cc -o a.out a.s
	./a.out

clean:
	rm -f cmp *.o *~ tmp* *.s *.out print

.PHONY: test clean run cmp print

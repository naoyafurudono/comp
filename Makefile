.DEFAULT_GOAL := cmp
CFLAGS=-std=c11 -g
LIBS=$(wildcard ./lib/*.c)
OBJS=$(LIBS:.c=.o)
$(OBJS): src/cmp.h
.PHONY: test clean run cmp print

cmp: $(LIBS)
	cc -o cmp $(CFLAGS) ./bin/cmp.c $(LIBS)

test: cmp
	bash test.sh

clean:
	rm -f cmp *.o *~ tmp* *.s *.out print

print: $(LIBS)
	cc -o print $(CFLAGS) ./bin/print.c $(LIBS)
	./print "$(ARGS)"

run: cmp
	./cmp "$(ARGS)" > tmp.s
	cc -o tmp.out tmp.s
	./tmp.out

debug: cmp
	./cmp "$(ARGS)" > tmp.s
	cc -o tmp.out tmp.s
	lldb ./tmp.out
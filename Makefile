.DEFAULT_GOAL := cmp
CFLAGS=-std=c11 -g
SRCS=$(wildcard ./src/*.c)
OBJS=$(SRCS:.c=.o)

cmp: $(SRCS)
	cc -o cmp $(CFLAGS) $(SRCS)

$(OBJS): src/cmp.h
test: cmp
	./test.sh

run: cmp
	./cmp "$(ARGS)" > a.s
	cc -o a.out a.s
	./a.out

clean:
	rm -f cmp *.o *~ tmp* *.s *.out

.PHONY: test clean run cmp

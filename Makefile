.DEFAULT_GOAL := cmp
CFLAGS=-std=c11 -g

cmp: ./src/cmp.c
	cc -o cmp $(CFLAGS) ./src/cmp.c

test: cmp
	./test.sh

run: cmp
	./cmp "$(ARGS)" > a.s
	gcc -o a.out a.s
	./a.out

clean:
	rm -f cmp *.o *~ tmp* *.s *.out

.PHONY: test clean run cmp

CFLAGS=-std=c11 -g

cmp: ./src/cmp.c
	cc -o cmp $(CFLAGS) ./src/cmp.c
test: cmp
	./test.sh

clean:
	rm -f cmp *.o *~ tmp*

.PHONY: test clean

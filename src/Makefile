CFLAGS=-std=c11 -g

cmp: ./cmp.c
	cc -o cmp $(CFLAGS) ./cmp.c
test: cmp
	./test.sh

clean:
	rm -f cmp *.o *~ tmp*

.PHONY: test clean

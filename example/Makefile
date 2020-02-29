
all:
	gcc -O -Wall -W lpsd_test.c random.c -o lpsd_test -lm
	./lpsd_test 1000000 > test.dat

.PHONY: clean

clean:
	rm -f lpsd_test test.dat

all: shttp.h main.c
	gcc -g -Wall main.c -o main

clean:
	rm -rvf *.o main


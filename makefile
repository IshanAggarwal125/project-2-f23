main: main.c
	gcc -o main main.c -Wall -Werror -lpthread
clean:
	rm *.o main


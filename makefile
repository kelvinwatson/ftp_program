all: ftserver.c
	gcc -g -o ftserver ftserver.c

clean:
	$(RM) ftserver

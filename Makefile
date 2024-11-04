CCFLAGS=-Wall -std=c99

compile: words.c
	gcc $(CCFLAGS) words.c -o words

debug: words.c
	gcc $(CCFLAGS) words.c -o words -DDEBUG=1

clean:
	rm -rf words
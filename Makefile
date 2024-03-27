debug-compile:
	gcc -g -o lsh lsh.c

compile:
	gcc -o lsh lsh.c

doc:
	pandoc -i README.md -o README.pdf

test:
	./test-lsh.sh

clean:
	rm -f lsh *~
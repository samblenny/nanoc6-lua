distill: distill.c
	gcc -O2 -march=native -Wall -o distill distill.c

.PHONY: run
run: distill
	./distill

.PHONY: clean
clean:
	rm distill

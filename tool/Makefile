all: test math

test:
	gcc -o test TestCaseGenerator.c

math:
	gcc -o math Math.c ../lib/libmath.c ../lib/libfile.c ../lib/liblog.c

clean:
ifneq ($(wildcard test),)
	rm test;
endif
ifneq ($(wildcard math),)
	rm math;
endif
ifneq ($(wildcard *.in),)
	rm *in;
endif
ifneq ($(wildcard *.out),)
	rm *out
endif

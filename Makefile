CFLAGS= -std=c99 -g -Wall -Wpedantic -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-switch -Wno-return-type -Wunused-variable
CFLAGS+= $(cflags)

test : pc11test
	./$<

pc11test : pc11object.o pc11parser.o pc11io.o pc11test.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

pc11object.o : pc11object.[ch]
pc11parser.o : pc11parser.[ch] pc11object.h
pc11io.o : pc11io.[ch] pc11parser.h pc11object.h
pc11test.o : pc11test.c pc11io.h pc11parser.h pc11object.h


clean :
	rm *.o

count :
	wc pc11*[ch]

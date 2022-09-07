CFLAGS ?= -g -Wall -Wpedantic -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-switch -Wno-return-type -Wunused-variable -Wno-parentheses
CFLAGS += -std=c99
SRC = pc11*[ch] ppnarg.h

test : pc11test
	./$<

pc11test : pc11object.o pc11parser.o pc11io.o pc11test.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

pc11object.o : pc11object.[ch]
pc11parser.o : pc11parser.[ch] pc11object.h
pc11io.o     : pc11io.[ch]     pc11object.h pc11parser.h
pc11test.o   : pc11test.[ch]   pc11object.h pc11parser.h pc11io.h


clean :
	rm *.o pc11test.exe

count :
	wc -l -c -L $(SRC)
	cloc $(SRC)

.PHONY : test clean count

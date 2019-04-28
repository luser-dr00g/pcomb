CFLAGS= -std=c99 -g -Wall -Wpedantic -Wno-switch -Wno-return-type -Wno-unused-variable
CFLAGS+= $(cflags)

test : pc9
	./$<

clean :
	rm *.o

pc9 : pc9obj.o pc9fp.o pc9par.o pc9tok.o pc9syn.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

pc8 : pc8obj.o pc8fp.o pc8par.o pc8tok.o pc8syn.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

pc7_test : pc7_test.c pc7.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)


pc9_test :
	gsnd -dNOSAFER pc9re2.ps



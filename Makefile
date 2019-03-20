CFLAGS= -std=c99
CFLAGS+= $(cflags)

test : pc5_test
	./$<

pc5_test : pc5_test.c pc5.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

pc4_test : pc4_test.c pc4.o pc4re.o pc4eg.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

pc7_test :
	gsnd -dNOSAFER pc7re2.ps



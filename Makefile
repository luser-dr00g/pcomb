CFLAGS= -std=c99
CFLAGS+= $(cflags)

test : pc7_test
	./$<

pc7_test : pc7_test.c pc7.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)


pc9_test :
	gsnd -dNOSAFER pc9re2.ps



CFLAGS= -std=c99 -g
CFLAGS+= $(cflags)

test : pc8
	./$<

pc8 : pc8obj.o pc8fp.o pc8par.o pc8tok.o pc8syn.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

pc7_test : pc7_test.c pc7.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)


pc9_test :
	gsnd -dNOSAFER pc9re2.ps



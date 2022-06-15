CFLAGS= -std=c99 -g -Wall -Wpedantic -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-switch -Wno-return-type -Wunused-variable
CFLAGS+= $(cflags)

test : pc5_test
	./$<

test_pc9 : pc9
	echo abc j string | ./$<

pc9 : pc9obj.o pc9fp.o pc9par.o pc9tok.o pc9syn.o pc9form.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

pc8 : pc8obj.o pc8fp.o pc8par.o pc8tok.o pc8syn.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

pc7c_test : pc7_test.c pc7.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

pc7_test :
	gsnd -dNOSAFER pc7re2.ps

pc5_test : pc5_test.c pc5.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

pc4_test : pc4_test.c pc4.o pc4re.o pc4eg.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)



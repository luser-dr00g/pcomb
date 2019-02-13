pc4_test : pc4_test.c pc4.o pc4re.o
	$(CC) -std=c99 $(CFLAGS) -o $@ $^ $(LDLIBS)



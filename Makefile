default: libdumpsched.so

CFLAGS += -g
libdumpsched.so: CFLAGS += -fPIC

libdumpsched.so: dumpsched.c
	$(CC) -shared -o "$@" $(CFLAGS) $+ $(LDFLAGS) $(LDLIBS)

default: libdumpsched.so

LIBRUNT ?= $(HOME)/work/devel/librunt.git
CFLAGS += -I$(LIBRUNT)/include

CFLAGS += -g
libdumpsched.so: CFLAGS += -fPIC

libdumpsched.so: dumpsched.c
	$(CC) -shared -o "$@" $(CFLAGS) $+ $(LDFLAGS) $(LDLIBS)

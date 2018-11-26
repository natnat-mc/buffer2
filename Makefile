.PHONY: all clean mrproper

LDFLAGS = -shared
CFLAGS = -I/usr/include/lua5.3/
LIBS = -llua5.3
OPTS = -Wall -Wextra -fPIC

CC = gcc
AR = ar
OBJS = buffer2.o wrapper.o
NAME = buffer2

LLIB = $(NAME).so
CLIB = $(NAME).a

all: $(LLIB) $(CLIB)

test: $(LLIB)
	lua5.3 -l $(NAME)

debug: $(LLIB)
	valgrind lua5.3 -l $(NAME)

clean:
	rm -f *.o

mrproper: clean
	rm -f $(LLIB) $(CLIB)

$(LLIB): $(OBJS)
	$(CC) $(OPTS) $(LIBS) $(LDFLAGS) $^ -o $@

$(CLIB): buffer2.o
	$(AR) cr $@ $^

%.o: %.c
	$(CC) $(OPTS) $(LIBS) $(CFLAGS) -c $^ -o $@

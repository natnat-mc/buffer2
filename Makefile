.PHONY: all clean mrproper

LDFLAGS = -shared
CFLAGS = -I/usr/include/lua5.3/
LIBS = -llua5.3
OPTS = -Wall -Wextra -fPIC

CC = gcc
OBJS = buffer2.o wrapper.o
NAME = buffer2

all: $(NAME).so

test: $(NAME).so
	lua5.3 -l $(NAME)

debug: $(NAME).so
	valgrind lua5.3 -l $(NAME)

clean:
	rm -f *.o

mrproper: clean
	rm -f $(NAME).so

$(NAME).so: $(OBJS)
	$(CC) $(OPTS) $(LIBS) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(OPTS) $(LIBS) $(CFLAGS) -c $^ -o $@

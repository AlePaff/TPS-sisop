CFLAGS := -ggdb3 -O2 -Wall -Wextra -std=c11
CFLAGS += -Wmissing-prototypes -Wvla
CPPFLAGS := -D_DEFAULT_SOURCE
VFLAGS = --leak-check=full --show-leak-kinds=all --track-fds=yes

PROGS := pingpong primes find xargs
PRIME := primes 100
PING := pingpong

all: $(PROGS)

find: find.o
xargs: xargs.o
primes: primes.o
pingpong: pingpong.o

format: .clang-files .clang-format
	xargs -r clang-format -i <$<

clean:
	rm -f $(PROGS) *.o core vgcore.*

.PHONY: all clean format

#Valgrindea
valgrindp: all
	valgrind $(VFLAGS) ./$(PRIME)

valgrindping: all
	valgrind $(VFLAGS) ./$(PING)

#valgrindping: all
#	valgrind $(VFLAGS) ./$(XARGS)

#valgrindping: all
#	valgrind $(VFLAGS) ./$(FIND)

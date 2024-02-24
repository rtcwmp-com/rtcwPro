CC = gcc
CFLAGS = -pipe -fsigned-char -DNDEBUG -O6 -m32 \
-fcommon -fomit-frame-pointer -ffast-math -falign-loops=2 -falign-jumps=2 \
-falign-functions=2 -fno-strict-aliasing -fstrength-reduce -fPIC

LDFLAGS = -shared -ldl -lm

SOURCES = $(shell ls ui_*.c ../game/bg_misc.c ../game/bg_stats.c ../qcommon/q_math.c ../qcommon/q_shared.c)
MODULES = $(SOURCES:.c=.o)

all: gameclean clean ui.mp.i386.so

gameclean:
	rm -rf ../game/*.o

ui.mp.i386.so: $(MODULES)
	$(CC) $(CFLAGS) -o $@ $? $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.so

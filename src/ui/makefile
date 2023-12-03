CC = gcc
CFLAGS = -pipe -fsigned-char -DNDEBUG -O6 -mtune=pentiumpro -march=pentium -m32\
-fomit-frame-pointer -ffast-math -falign-loops=2 -falign-jumps=2 \
-falign-functions=2 -fno-strict-aliasing -fstrength-reduce -fPIC

SOURCES = $(shell ls ui_*.c ../game/bg_misc.c ../game/bg_stats.c ../qcommon/q_math.c ../qcommon/q_shared.c)
MODULES = $(SOURCES:.c=.o)

all: gameclean ui.mp.i386.so

gameclean:
	rm -rf ../game/*.o

ui.mp.i386.so: $(MODULES)
	$(CC) -shared -m32 -ldl -lm -o $@ $?

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.so

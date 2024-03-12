CC = gcc


CFLAGS = -pipe -fsigned-char -DNDEBUG -O6 -m32 \
-fcommon -fomit-frame-pointer -ffast-math -falign-loops=2 -falign-jumps=2 \
-falign-functions=2 -fno-strict-aliasing -fstrength-reduce -fPIC -DCGAMEDLL

LDFLAGS = -shared -ldl -lm

SOURCES = $(shell ls cg_*.c ../game/bg_animation.c ../game/bg_misc.c ../game/bg_pmove.c ../game/bg_slidemove.c ../qcommon/q_math.c ../qcommon/q_shared.c ../ui/ui_shared.c)
MODULES = $(SOURCES:.c=.o)



all: clean gameclean cgame.mp.i386.so

gameclean:
	rm -rf ../qcommon/q_math.o
	rm -rf ../qcommon/q_shared.o
	rm -rf ../game/*.o ../ui/*.o

cgame.mp.i386.so: $(MODULES)
	$(CC) $(CFLAGS) -o $@ $? $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.so

CC = gcc -m32


CFLAGS = -pipe -fsigned-char -DNDEBUG -O6 -mtune=pentiumpro -march=pentium \
-fomit-frame-pointer -ffast-math -falign-loops=2 -falign-jumps=2 \
-falign-functions=2 -fno-strict-aliasing -fstrength-reduce -fPIC -DCGAMEDLL

SOURCES = $(shell ls cg_*.c ../game/bg_animation.c ../game/bg_misc.c ../game/bg_pmove.c ../game/bg_slidemove.c ../game/q_math.c ../game/q_shared.c ../ui/ui_shared.c)
MODULES = $(SOURCES:.c=.o)



all: clean gameclean cgame.mp.i386.so

gameclean:
	rm -rf ../game/*.o ../ui/*.o

cgame.mp.i386.so: $(MODULES)
	$(CC) -shared -ldl -lm -o $@ $?

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.so

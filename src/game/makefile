CC = gcc


CFLAGS =  -pipe -fsigned-char -DNDEBUG -O6 -m32 \
-fcommon -fomit-frame-pointer -ffast-math -falign-loops=2 -falign-jumps=2 \
-falign-functions=2 -fno-strict-aliasing -fstrength-reduce -fPIC -DGAMEDLL -I../../deps/jansson/include

LDFLAGS =  -shared -ldl -lm -L../../deps/jansson/build/lib

SOURCES = $(shell ls  g_*.c ../qcommon/q_math.c ../qcommon/q_shared.c ai_*.c bg_animation.c bg_misc.c bg_pmove.c bg_stats.c bg_slidemove.c ../botai/*.c  ../qcommon/crypto/sha-1/sha1.c )
MODULES = $(SOURCES:.c=.o)

all: gameclean qagame.mp.i386.so

gameclean:
	rm -rf *.o ../botai/*.o
	rm -rf ../qcommon/crypto/sha-1/*.o
	rm -rf ../qcommon/*.o

qagame.mp.i386.so: $(MODULES)
	$(CC) $(CFLAGS) -o $@ $? ../../deps/jansson/build/lib/libjansson.a $(LDFLAGS)


.c.o:
	$(CC) $(CFLAGS) -c $< -o $@  $(LDFLAGS) 


clean:
	rm -f *.o *.so ../botai/*.o

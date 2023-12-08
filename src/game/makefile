CC = gcc


CFLAGS =  -pipe -fsigned-char -DNDEBUG -O6 -mtune=pentiumpro -march=pentium -m32 \
-fomit-frame-pointer -ffast-math -falign-loops=2 -falign-jumps=2 \
-falign-functions=2 -fno-strict-aliasing -fstrength-reduce -fPIC -DGAMEDLL -I./../qcommon/jansson/

LDFLAGS = -I./../qcommon/jansson 
#-ljansson

SOURCES = $(shell ls  ../qcommon/jansson/*.c g_*.c ../qcommon/q_math.c ../qcommon/q_shared.c ai_*.c bg_animation.c bg_misc.c bg_pmove.c bg_stats.c bg_slidemove.c ../botai/*.c  ../qcommon/crypto/sha-1/sha1.c )
#SOURCES = $(shell ls g_*.c q_*.c ai_*.c bg_animation.c bg_misc.c bg_pmove.c bg_stats.c bg_slidemove.c ../botai/*.c  ../qcommon/crypto/sha-1/sha1.c )
MODULES = $(SOURCES:.c=.o)



all: gameclean qagame.mp.i386.so

gameclean:
	rm -rf *.o ../botai/*.o
	rm -rf ../qcommon/crypto/sha-1/*.o

qagame.mp.i386.so: $(MODULES)
	$(CC)  -shared -m32 -ldl -lm -o $@ $? $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@  $(LDFLAGS) 

clean:
	rm -f *.o *.so ../botai/*.o

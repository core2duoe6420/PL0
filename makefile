CC=gcc
CFLAGS=-std=gnu99 -c -D__LINUX__ 
SOURCES=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SOURCES))

PL0:$(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

-include $(SOURCES:.c=.d)

%.d:%.c
	$(CC) -MM $(CFLAGS) $< >$@.$$$$;\
	sed 's,\($*\).o[ :]*,\1.o $@:,g' < $@.$$$$ >$@;\
	rm -f $@.$$$$

.PHONY:clean
clean:
	rm -f *.o *.d *.out

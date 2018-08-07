SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)
DEPS = $(SRCS:%.c=.%.d)

CFLAGS = -g3 -Wall

.PHONY: glshell doc
all: glshell

doc:
	doxygen

glshell: $(OBJS)
	$(CC) -o $@ $(OBJS)

.%.d: %.c
	$(CC) -MM -o $@ $^

include $(DEPS)

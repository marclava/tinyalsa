CROSS_COMPILE ?=

CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Wextra -Werror -Wfatal-errors -I ../include

VPATH = ../src

EXAMPLES += pcm-readi
EXAMPLES += pcm-readi-mmap
EXAMPLES += pcm-writei

.PHONY: all
all: $(EXAMPLES)

pcm-readi pcm-mmap-readi pcm-writei: LDLIBS+=-ldl

pcm-readi: pcm-readi.c -ltinyalsa

pcm-readi-mmap: pcm-readi-mmap.c -ltinyalsa

pcm-writei: pcm-writei.c -ltinyalsa

.PHONY: clean
clean:
	rm -f $(EXAMPLES)


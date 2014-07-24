
CC = gcc
LD = ld
RM = rm
MKDIR = mkdir
INSTALL = install
PREFIX = /usr

CFLAGS = -O2 -fPIC -I../FaceRecEm/common/src -fno-stack-protector -g

CFLAGS += `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv` libneven.so

OBJS = test.o

all: test

test: $(OBJS)
	$(CC) $^ $(LIBS) -o $@

clean:
	$(RM) -f $(OBJS) test


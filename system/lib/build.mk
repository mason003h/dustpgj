LIB_OBJS = lib/src/syscall.o \
	lib/src/stdlib.o \
	lib/src/pmessage.o \
	lib/src/stdio.o \
	lib/src/cmain.o \
	lib/src/unistd.o \
	lib/src/dev.o \
	lib/src/vfs.o \
	lib/src/proto.o \
	lib/src/package.o \
	lib/src/kserv/kserv.o \
	lib/src/kserv/fs.o \
	lib/src/kserv/userman.o

lib/libewoklibc.a: $(LIB_OBJS)
	mkdir -p build
	$(AR) rT $@ $(LIB_OBJS)	

EXTRA_CLEAN += $(LIB_OBJS) lib/libewoklibc.a

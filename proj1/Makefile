UNAME := $(shell uname)

DEBUG=1

SLIB=lib739kv.so

VPATH=./src/:./testing

OBJDIR=./obj/


CC=gcc

CPP=g++ -std=c++11

OPTS=-Ofast

LDFLAGS= -lm -pthread -ldl

COMMON= -Iinclude/ -Isrc/

CFLAGS=-Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC

CFLAGS_PYTHON = `pkg-config --cflags python3`

LDFLAGS_PYTHON = `pkg-config --libs python3`

ifeq ($(DEBUG), 1)
	OPTS=-O0 -g
else
	OPTS=-Ofast -DNDEBUG 
endif


#for MacOS clang we need to add libc++
ifeq ($(UNAME),Darwin)
	CPP += -stdlib=libc++
endif


CFLAGS+=$(OPTS)


#list of objects to build
OBJ=	sqlite3.o \
	    debug.o \
		sqlstatement.o \
		datastore.o \
		message.o \
		dsserver.o \
		dsclient.o \
		kvproxy.o \
		lib739kv.o \
		rpcclient.o \
		rpcserver.o \


#list of executable to build
EXEC =  kvdaemon \
		test \
		server_test \
		client_test \
		api_test \
		rpcc \
		rpcs \


EXECOBJA = $(addprefix $(OBJDIR), $(EXEC))

EXECOBJ = $(addsuffix .o, $(EXECOBJA))

OBJS = $(addprefix $(OBJDIR), $(OBJ))

DEPS = $(wildcard src/*.h) Makefile include/lib739kv.h

TESTFILES = test*.db

all: obj $(SLIB) $(EXECOBJ) $(EXEC) python_binding TAGS

$(SLIB): $(OBJS)
	$(CPP) $(CFLAGS) -shared $^ -o $@ $(LDFLAGS)

$(OBJDIR)%.o: %.cpp $(DEPS)
	$(CPP) $(COMMON) $(CFLAGS) -c $< -o $@

$(OBJDIR)%.o: %.c $(DEPS)
	$(CC) $(COMMON) $(CFLAGS) -c $< -o $@

$(EXEC): $(EXECOBJ) $(SLIB)
	$(CPP) $(COMMON) $(CFLAGS) $(OBJDIR)$@.o -o $@ $(LDFLAGS) $(SLIB)

kvs739.o: kvs739.cpp $(DEPS)
	$(CPP) $(COMMON) $(CFLAGS) $(CFLAGS_PYTHON) -c $< -o $(addprefix $(OBJDIR), $@)

python_binding: $(SLIB) kvs739.o
	$(CPP) -shared $(addprefix $(OBJDIR), kvs739.o)	$(LDFLAGS) $(SLIB) $(LDFLAGS_PYTHON) -o kvs.so
	cp *.so ./testing

TAGS: $(wildcard src/*.h) $(wildcard src/*.cpp) $(wildcard testing/*.h) $(wildcard testing/*.cpp) Makefile include/lib739kv.h
	-find . -type f -iname "*.[chS]p?" | xargs etags -a

obj:
	mkdir -p obj

.PHONY: clean

clean:
	rm -rf $(OBJS) $(SLIB) $(EXEC) $(EXECOBJ) $(OBJDIR) $(TESTFILES) ./testing/*.so kvs.so TAGS

os := $(shell uname)
server = server
client = client
compiler = g++
flags = -W -Wall -pedantic
dflags = -g -DDEBUG -DUSE_DEBUG
lib = -lboost_program_options-mt -lpthread
cmp = $(compiler) $(flags) $(inc) -c
lnk = $(compiler) $(flags) $(lib) -o $(bin)
objects = server.o eventbase.o network.o tpool.o

ifeq ($(os), Darwin)
    flags += -j8
endif

all : $(server) $(client)

debug : flags += $(dflags)
debug : $(server) $(client)

$(client) : bin = $(client)
$(client) : client.o
	$(lnk) client.o network.o

client.o : client.cpp network.hpp
	$(cmp) client.cpp
	
network.o : network.cpp network.hpp
	$(cmp) network.cpp

tpool.o : tpool.c tpool.h
	$(cmp) tpool.c

$(server) : bin = $(server)
$(server) : lib += -levent -levent_pthreads
$(server) : $(objects)
	$(lnk) $(objects)

server.o : server.cpp
	$(cmp) server.cpp

eventbase.o : eventbase.cpp eventbase.hpp network.hpp
	$(cmp) eventbase.cpp

clean :
	rm $(server) $(client) *.o

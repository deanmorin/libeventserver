server = server
client = client
compiler = g++
flags = -W -Wall -pedantic -j8
dflags = -g -DDEBUG -DUSE_DEBUG
inc = -I/usr/local/Cellar/boost/1.48.0/include/boost
lib = -lboost_program_options-mt
cmp = $(compiler) $(flags) $(inc) -c
lnk = $(compiler) $(flags) $(lib) -o $(bin)
objects = server.o eventbase.o network.o tpool.o

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
	rm $(server) $(client) $(objects)

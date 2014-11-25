# Makefile for server

CC = gcc
OS = $(shell uname -s)
WARN = -w
#WARN = -Wall

ifeq (${OS}, SunOS)
	THREADFLAG=-pthreads -lnsl -lsocket -lresolv
else
	THREADFLAG=-pthread
endif

all: server

server:
	${CC} ${WARN} -g ${THREADFLAG} -o server server.c socket_server.c socket_register.c server_stub_receive.c ../lib/libsocket.c ../lib/getaddrinfo.c ../lib/liblog.c ../lib/libfile.c ../lib/libterminal.c ../lib/libmath.c ../packet/register.c ../lib/libscientific.c ../packet/checksum.c ../packet/execute_ack.c ../packet/linkseq.c ../packet/linkload.c ../packet/execute.c ../packet/execute_reply.c ../packet/register_ack.c ../packet/hello.c ../lib/rtt.c

clean:
ifneq ($(wildcard ../.1*),) 
	rm ../.1*; 
endif
ifneq ($(wildcard server),)
	rm server;
endif
ifneq ($(wildcard SRPC.log),)
	rm SRPC.log;
endif

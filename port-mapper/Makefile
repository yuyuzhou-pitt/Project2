# Makefile for port-mapper

CC = gcc
OS = $(shell uname -s)
WARN = -w # -Wall


ifeq (${OS}, SunOS)
	THREADFLAG=-pthreads -lnsl -lsocket -lresolv
else
	THREADFLAG=-pthread
endif

all:port-mapper

port-mapper:
	${CC} ${WARN} -g ${THREADFLAG} -o port-mapper port_mapper.c socket_port_mapper.c ../lib/libsocket.c ../lib/getaddrinfo.c ../lib/liblog.c ../lib/libfile.c ../lib/libterminal.c ../packet/register.c ../packet/register_ack.c ../packet/request.c ../packet/request_reply.c ../packet/linkload.c ../lib/libscientific.c ../packet/checksum.c

clean:
ifneq ($(wildcard port-mapper),)
	rm port-mapper;
endif
ifneq ($(wildcard SRPC.log),)
	rm SRPC.log;
endif
ifneq ($(wildcard ../.port_mapper*),) 
	rm ../.port_mapper*; 
endif

# Makefile for server

CC = gcc
OS = $(shell uname -s)
WARN = -w # -Wall

ifeq (${OS}, SunOS)
	THREADFLAG=-pthreads -lnsl -lsocket -lresolv
else
	THREADFLAG=-pthread
endif

all:minigoogle client

minigoogle:
	${CC} ${WARN} -g ${THREADFLAG} -o minigoogle minigoogle.c socket_request.c socket_execute.c client_stub_send.c client_stub.c ../lib/libsocket.c ../lib/getaddrinfo.c ../lib/liblog.c ../lib/libfile.c ../lib/libterminal.c ../lib/libmath.c ../packet/register.c ../packet/request.c ../packet/request_reply.c ../packet/linkload.c  ../packet/linkseq.c ../lib/libscientific.c ../packet/execute.c ../packet/execute_ack.c ../packet/execute_result.c ../packet/checksum.c ../lib/rtt.c ../packet/mapreduce.c ../lib/libmapreduce.c ../lib/libmr.c ../packet/linkmii.c

client:
	${CC} ${WARN} -g ${THREADFLAG} -o client client.c socket_request.c socket_execute.c client_stub_send.c client_stub.c ../lib/libsocket.c ../lib/getaddrinfo.c ../lib/liblog.c ../lib/libfile.c ../lib/libterminal.c ../lib/libmath.c ../packet/register.c ../packet/request.c ../packet/request_reply.c ../packet/linkload.c  ../packet/linkseq.c ../lib/libscientific.c ../packet/execute.c ../packet/execute_ack.c ../packet/execute_result.c ../packet/checksum.c ../lib/rtt.c ../packet/mapreduce.c ../lib/libmapreduce.c ../lib/libmr.c ../packet/linkmii.c

clean:
ifneq ($(wildcard client),)
	rm minigoogle;
	rm client;
endif
ifneq ($(wildcard SRPC.log),)
	rm SRPC.log;
endif
ifneq ($(wildcard ../*.out),)
	rm ../*.out;
endif

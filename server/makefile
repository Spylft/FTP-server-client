main: server.o status.o command.o message_transport.o file_transport.o
	gcc server.o status.o command.o message_transport.o file_transport.o -o server
command: command.c command.h
	gcc command.c -c command.o
file_transport: file_transport.c file_transport.h
	gcc file_transport.c -c file_transport.o
message_transport: message_transport.c message_transport.h
	gcc message_transport.c -c message_transport.o
server: server.c 
	gcc server.c -c server.o
status: status.c status.h
	gcc status.c -c status.o

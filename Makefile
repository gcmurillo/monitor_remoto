CC = gcc
CFLAGS = -Wall
OBJS = ./obj/csapp.o
DIR = ./include/ 

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread -L ./lib/ -Wl,-rpath=../lib/ -lcbor 

all: client server

client: ./src/client.c ./obj/csapp.o
	if [ ! -d "./bin" ];then \
        	mkdir bin; \
    	fi
	$(CC) $(CFLAGS) -o ./bin/monitor-client ./src/client.c $(OBJS) $(LIB) -I $(DIR)
	@echo 'Creado ejecutable monitor-client en la carpeta bin'

server: ./src/server.c ./obj/csapp.o
	$(CC) $(CFLAGS) -o ./bin/monitor-server ./src/server.c $(OBJS) $(LIB) -I $(DIR)
	@echo 'Creado ejecutable monitor-server en la carpeta bin'

./obj/csapp.o:
	if [ ! -d "./obj" ];then \
        	mkdir obj; \
    	fi
	$(CC) $(CFLAGS) -c ./src/csapp.c -o ./obj/csapp.o -I $(DIR)

clean:
	rm -rf obj bin *~ log.txt
	@echo 'limpio'


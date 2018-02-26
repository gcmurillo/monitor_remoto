#include "server.h"
#include <getopt.h>
#include <sys/resource.h>

int hflag = 0; //Opcion -h para ayuda
int cflag = 0; //Opcion -c para consola
int lflag = 0; //Opcion -l para cambiar archivo de log

//cbor
cbor_item_t * root ;
unsigned char * bufferCbor;
size_t buffer_size;
size_t tamano;
char buffer[MAXLINE];
FILE * filog;



int main(int argc, char **argv)
{
	
	
	//leerArchivos();
	
	int c;
	char *path;
	int listenfd, *connfdp;
	char *haddrp, *port;
	pthread_t tid;
	struct hostent *hp;
	unsigned int clientlen;
	struct sockaddr_in clientaddr;
	int n_argumentos = argc;
	
	char ultimo = argv[n_argumentos - 1];
	int fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	char tiempoStr[50];
	char tiempostr2[50];
	int logfd;
	
	umask(0111);
	
	// loop que permite obtener los diferentes flags 
	while ((c = getopt(argc, argv, "hcl:")) != -1)
	{
		switch(c)
		{
			case 'h':
				hflag = 1;
				break;

			case 'c':
				cflag = 1;
				break;

			case 'l':
				lflag = 1;
				path = optarg;
		}

	}
	// flag de ayuda
	if (hflag){
		printf("Opciones:\n");
		printf("\t-h: ayuda (este mensaje)\n");
		printf("\t-c: modo consola\n");
		printf("\t-l: archivo de log\n");
		exit(0); //si coloca -h, solo mostrar ayuda,
			 //no ejecutar funcionalidad
	}
	//flag de modo consola
	if (cflag){
		printf("Modo consola\n");
	}
	//modo daemon
	else{
		if ((pid = fork()) < 0)
			fprintf(stderr, "fork erroneo %s\n", argv[0]);
		else if (pid != 0)
			exit(0);
		setsid();

		if (rl.rlim_max == RLIM_INFINITY)
			rl.rlim_max = 1024;
		for(int i = 0; i < rl.rlim_max; i++)
			close(i);

		close(0);
		close(1);
		close(2);
		fd0 = open("/dev/null", O_RDWR);
		fd1=dup(0);
		fd2 = dup(0);
		if (lflag){
			printf("Archivo de log se guardara en: %s\n", path);
			logfd = open(path, O_CREAT | O_RDWR | O_APPEND , 0777);
			filog = Fdopen(logfd ,"r+");
		
		}else{
			logfd = open("../log.txt", O_CREAT | O_RDWR | O_APPEND , 0777);
			filog = Fdopen(logfd ,"r+");
		}
	}
	
	if (n_argumentos == 1){
		listenfd = Open_listenfd("8080");
	}else{
		port = argv[n_argumentos - 1];
		listenfd = Open_listenfd(port);
	}
	
	// loop que permite la conexion de multiples clientes creando un hilo para cada uno 
	while (1) {
		clientlen = sizeof(clientaddr);
		
		connfdp = Malloc(sizeof(int));
		*connfdp = Accept(listenfd, (SA *) &clientaddr, &clientlen);
		Pthread_create(&tid, NULL, thread, connfdp);
		 //Determine the domain name and IP address of the client 
		hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
					sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		haddrp = inet_ntoa(clientaddr.sin_addr);
		time_t clk = time(NULL);
		ctime_r(&clk, tiempoStr);
		

		strcpy(tiempostr2,tiempoStr);
		tiempostr2[strlen(tiempostr2)-1] = 0;
		
		printf("[%s] server connected to %s (%s)\n",tiempostr2, hp->h_name, haddrp);
		// if que permite la escritura en el archivo de log
		if(lflag == 1 && cflag == 0){
			printf("escribientdo\n");
			fprintf(filog, "[%s] server connected to %s (%s)\n",tiempostr2, hp->h_name, haddrp );
			fflush(filog);
		}
		if (cflag == 0 && lflag == 0){
			fprintf(filog, "[%s] server connected to %s (%s)\n",tiempostr2, hp->h_name, haddrp );
			fflush(filog);
		}

	}
	exit(0);
}
//hilo que se crea por cada conexion que reciba el server 
void *thread(void *vargp)
{
	int connfd = *((int *)vargp);
	Pthread_detach(pthread_self());
	Free(vargp);
	echo(connfd);
	Close(connfd);
	return NULL;
}
/*funcion que escribe el buffer que contiene el string codificado con cbor
ademas de manejar el problema cuando el cliente se desconecta el servidor escribe sobre 
un socket sin que nadie este escuchando
*/
void echo(int connfd)
{
	int escrito, leido;
	char buf[MAXLINE];
	rio_t rio;
	Rio_readinitb(&rio, connfd);
	
	while(1){
		
		leerArchivos();
		//fwrite(bufferCbor, 1, tamano, stderr);
		
		if ((leido =Rio_readlineb(&rio, buf, 3)) != 2 )
		{
			if(lflag == 1 && cflag == 0){
				fprintf(filog, "%s", "desconexion\n");
				fflush(filog);
				return;
			}
			if (cflag == 0 && lflag == 0){
				fprintf(filog, "%s", "desconexion\n");
				fflush(filog);
				return;
			}		
			printf("%s", "desconexion\n");
			return; 
		}
		//printf("%s\n", buf);
		escrito = rio_writen(connfd,bufferCbor,tamano);
		free(bufferCbor);
		cbor_decref(&root);
		sleep(2);
	}
	
}
// lecutara y creacion del objeto de cbor correspondiente a la version del sistema
void leerArchivoVersion(){
	int fd ;
	rio_t rio;
	char data[MAXLINE];
	char *file = "/proc/version_signature";
	fd = Open(file,O_RDONLY,0);
	FILE *fil = Fdopen(fd ,"r");
	Rio_readinitb(&rio, fd);
	Rio_readlineb(&rio, data, MAXLINE);
	cbor_map_add(root, (struct cbor_pair) {
		.key = cbor_move(cbor_build_string("Sistema")),
		.value = cbor_move(cbor_build_string(data))
	});
	strcpy(buffer, data);
	rewind(fil);
	Close(fd);
	
}
// lecutara y creacion del objeto de cbor correspondiente al uptime 
void leerArchivoUptime(){
	int fd;
	int tie;
	rio_t rio;
	
	char file[] ="/proc/uptime";
	struct stat buf;
	char data[MAXLINE];
	if(stat(file,&buf) < 0){
		fprintf(stderr, "Error al leer el archivo %s\n", file );
	}else{
		
		fd = Open(file,O_RDONLY,0);
		FILE *fil = Fdopen(fd ,"r");
		Rio_readinitb(&rio, fd);
		Rio_readlineb(&rio, data, MAXLINE);
		sscanf(data, "%d", &tie);
		cbor_map_add(root, (struct cbor_pair) {
		.key = cbor_move(cbor_build_string("tiempo")),
		.value = cbor_move(cbor_build_uint32(tie))
		});
		rewind(fil);
		Close(fd);
	}



}
// lecutara y creacion del objeto de cbor correspondiente a la imformacion de los procesos
void leerArchivoStat(){
	int fd,n,n1,n2;
	//int contEntrada = 0 ;
	int contador =0 ;
	char proc[15];
	int numProc;
	rio_t rio;
	FILE *fil;
	char buf1[MAXLINE];
	char file[] ="/proc/stat";
	struct stat buf;
	if(stat(file,&buf) < 0){
		fprintf(stderr, "Error al leer el archivo %s\n", file );
	}else{
		fd = Open(file,O_RDONLY,0);
		fil = Fdopen(fd ,"r");
		Rio_readinitb(&rio, fd);
		while((n = Rio_readlineb(&rio, buf1, MAXLINE)) != 0) {
			if(buf1[0] == 'c' && buf1[1] == 'p'){
				strcat(buffer,buf1);
				if(sscanf(buf1,"cpu %d",&n1) == 1)
					if(n1 != 0){
						cbor_map_add(root, (struct cbor_pair) {
						.key = cbor_move(cbor_build_string("total")),
						.value = cbor_move(cbor_build_uint32(n1))
						});

					}
				if(sscanf(buf1,"cpu0 %d",&n2) == 1)								//Agregrar para que cuente mas de 1 cpu
					if(n2 != 0){
						cbor_map_add(root, (struct cbor_pair) {
						.key = cbor_move(cbor_build_string("cpu0")),
						.value = cbor_move(cbor_build_uint32(n2))
						});
						contador = contador +1 ;
					}
			}
			if(buf1[0] == 'p' && buf1[1] == 'r' ){
				strcat(buffer, buf1);
				sscanf(buf1, "%s %d", proc , &numProc);
				if(numProc == 0)
					numProc = 1 ;
				cbor_map_add(root, (struct cbor_pair) {
				.key = cbor_move(cbor_build_string(proc)),
				.value = cbor_move(cbor_build_uint32(numProc))
				});
			}
		}
		cbor_map_add(root, (struct cbor_pair) {
		.key = cbor_move(cbor_build_string("NumeroCpus")),
		.value = cbor_move(cbor_build_uint8(contador))
		});
	}	
	rewind(fil);
	Close(fd);

}
// lecutara y creacion del objeto de cbor correspondiente a la memoria usada
void leerArchivoMemInfo(){
	int fd , memoria;
	rio_t rio;
	char str[10];
	char buf1[MAXLINE];
	char file[] ="/proc/meminfo";
	struct stat buf;
	if(stat(file,&buf) < 0){
		fprintf(stderr, "Error al leer el archivo %s\n", file );
	}else{
		fd = Open(file,O_RDONLY,0);
		FILE *fil = Fdopen(fd ,"r");
		Rio_readinitb(&rio, fd);
		Rio_readlineb(&rio, buf1, MAXLINE);
		sscanf(buf1, "%s %d kB", str,&memoria);
		cbor_map_add(root, (struct cbor_pair) {
		.key = cbor_move(cbor_build_string(str)),
		.value = cbor_move(cbor_build_uint32(memoria))
		});
		strcat(buffer,buf1);
		Rio_readlineb(&rio, buf1, MAXLINE);
		sscanf(buf1,"%s %d kB",str,&memoria);
		cbor_map_add(root, (struct cbor_pair) {
		.key = cbor_move(cbor_build_string(str)),
		.value = cbor_move(cbor_build_uint32(memoria))
		});
		strcat(buffer, buf1);
		rewind(fil);
		Close(fd);	
		
	}
}


void handler(int sig) {
		
		//strcpy(buffer," ");
		leerArchivos();
		strcpy(buffer," ");
		Alarm(5);
}
//llamada a la lectura de los archivos y creacion del mapa que se codificara con cbor 
void leerArchivos(){
	root = cbor_new_definite_map(10);
	leerArchivoVersion();
	leerArchivoUptime();
	leerArchivoStat();
	leerArchivoMemInfo();
	tamano = cbor_serialize_alloc(root, &bufferCbor, &buffer_size);
	
	
}

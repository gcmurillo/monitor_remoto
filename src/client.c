#include "csapp.h"
#include "cbor.h"
#include <stdlib.h>

struct cbor_load_result result;
int formatTime(int segundos);
int flag = 1 ;
void handler(int sig);
int main(int argc, char **argv)

{
	int clientfd,n;
	size_t tamano ;
	char *port;
	char *host;
	unsigned char buf[MAXLINE];
	rio_t rio;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
		exit(0);
	}
	host = argv[1];
	port = argv[2];
	Signal(2,handler);
	clientfd = Open_clientfd(host, port);
	Rio_readinitb(&rio, clientfd);
	setbuf(stdout,NULL);
	// loop con un flag que cambiara cuando se precione la combinacion de teclas ctrol + c
	// y notificara al servidor de la desconexion 
	while(flag) {
			if(flag){
				rio_writen(clientfd,"a\n",2);
				
			}
			tamano = read(clientfd, buf, MAXLINE);
			printf("\n--------------------------------------\n" );
			//fwrite(buf, 1, tamano, stdout);
	    	cbor_item_t * item2 = cbor_load(buf, tamano, &result);
	    	if (result.error.code != CBOR_ERR_NONE) 
				printf("There was an error while reading the input near byte %zu (read %zu bytes in total): ", result.error.position, result.read);
	    	switch (result.error.code) {
				case CBOR_ERR_MALFORMATED:
				{
					printf("Malformed data\n");
					break;
				}
				case CBOR_ERR_MEMERROR:
				{
					printf("Memory error -- perhaps the input is too large?\n");
					break;
				}
				case CBOR_ERR_NODATA:
				{
					printf("The input is empty\n");
					break;
				}
				case CBOR_ERR_NOTENOUGHDATA:
				{
					printf("Data seem to be missing -- is the input complete?\n");
					break;
				}
				case CBOR_ERR_SYNTAXERROR:
				{
					printf("Syntactically malformed data -- see http://tools.ietf.org/html/rfc7049\n");
					break;
				}
				case CBOR_ERR_NONE:
				{
					// GCC's cheap dataflow analysis gag
					break;
				}
			}
			system("clear");
			cbor_item_t * item3 = cbor_map_handle(item2)[0].key;
	   		//printf("\r%s  :", cbor_string_handle(item3));
	    	// decodficacion del string de cbor y posterior print en la consola 
			cbor_item_t * item5 = cbor_map_handle(item2)[0].value;
	   		printf("\r%s  :  %s", cbor_string_handle(item3), cbor_string_handle(item5));
			cbor_item_t * valor ;
	   		cbor_item_t * llave ;
	   		for (size_t i = 1; i < 10; i++) {
   				if( i == 1){
   					llave = cbor_map_handle(item2)[i].key;
			
					valor = cbor_map_handle(item2)[i].value;
					printf("\r%s  :", cbor_string_handle(llave));
					formatTime(cbor_get_int(valor) );
   				}else{
					llave = cbor_map_handle(item2)[i].key;
			
					valor = cbor_map_handle(item2)[i].value;
					printf("\r%s   :   %d\n", cbor_string_handle(llave), cbor_get_int(valor) );
			}
		}




			printf("%d-------------------\n",strlen(buf) );
			//strcpy(buf," ");
			//rewind(stdout);
			//Free(buf);
			
	}
	
	Close(clientfd);
	exit(0);
}
// funcion que formatea los segundos a horas minutos y segundos 
int formatTime(int segundos){
	int horas, min, seg ;
	if(segundos>3600){
		min = segundos/60 ;
		seg = segundos%60;
		horas = min/60;
		min = min%60;
		printf(" %d h  %d min  %d seg\n", horas,min,seg );
	

	}else{
		min = segundos/60;
		seg = segundos%60; 
		printf("minutos: %d segundos :%d\n", min,seg );
	}

}
void handler(int sig) {
		
		flag = 0 ;
}

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "csapp.h"
#include <time.h>
#include <signal.h>
#include <cbor.h>



void leerArchivoVersion();
void leerArchivoUptime();
void leerArchivoStat();
void leerArchivoMemInfo();
void leerArchivos();
void handler();
void echo(int connfd);
void *thread(void *vargp);

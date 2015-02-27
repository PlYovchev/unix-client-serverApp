/*
 ============================================================================
 Name        : ProjectOS_Client_Server.c
 Author      : me
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "StartThread.h"

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
//	if(argc < 1){
//		error("No port provided!");
//	}
	puts("!!!Hello! Server started!!!"); /* prints !!!Hello World!!! */
	printf("%d\n",argv[1]);
	//startServerThread(5702);
	startServer(5702);
	return EXIT_SUCCESS;
}

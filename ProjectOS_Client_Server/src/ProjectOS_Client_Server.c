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

int main(void) {
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	//startServerThread(5678);
	startServer(5691);
	return EXIT_SUCCESS;
}

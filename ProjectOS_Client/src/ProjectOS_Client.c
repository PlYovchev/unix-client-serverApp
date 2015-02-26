/*
 ============================================================================
 Name        : ProjectOS_Client.c
 Author      : me
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    portno = 5691;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
//    if (n < 0)
 //        error("ERROR writing to socket");

    while(1){
    	bzero(buffer,256);
    	receiveMessage(sockfd, buffer, 256);

    	if(checkForCommand(buffer) == 1){
    		char message[256];
			bzero(message,256);
			scanf("%s", &message);
			processTheMessageToServer(sockfd, message);
    	}
    	else if(checkForCommand(buffer) == 2){
    		char message[256];
			bzero(message,256);
			scanf("%s", &message);
			processTheMessageToServer(sockfd, message);
			if(strcmp(message, "y") == 0){
				bzero(message,256);
				scanf("%s", &message);
				processTheFileToServer(sockfd,message);
			}
    	}
    	else{
    		puts(buffer);
    	}
    }

    close(sockfd);
    return 0;
}

int checkForCommand(char* message){
	int shouldItWaitForReponse = 0;
	if(strcmp(message, "RegLog") == 0 || strcmp(message, "enterUsr") == 0
			|| strcmp(message, "enterPass") == 0){
		shouldItWaitForReponse = 1;
	}

	if(strcmp(message, "sndFQ") == 0){
		shouldItWaitForReponse = 2;
	}

	return shouldItWaitForReponse;
}

void receiveMessage(int serverSocket, char* realMessage, int sizeOfBuffer){
	int atChar = 0;
	while(1){
		int endProcessing = 0;
		char buffer[13];
		char code[2];
		char size;
		bzero(buffer, 13);
		bzero(code, 2);
		read(serverSocket, buffer, 13);
		char* msgToParse = buffer;
		memcpy(code, msgToParse, 2);
		msgToParse += 2;
		size = msgToParse[0];
		msgToParse += 1;

		if(code[1] == 'a'){
			int realSize = size - 48;
			char partOfMsg[realSize];
			bzero(partOfMsg, realSize);
			memcpy(partOfMsg, msgToParse, realSize);
			int i = 0;
			for (i = atChar; i < atChar + realSize; i++) {
				realMessage[i] = partOfMsg[i - atChar];
			}
			endProcessing = 1;
			atChar += realSize;
		}
		else if(code[1] == 'e'){
			char partOfMsg[10];
			bzero(partOfMsg, 10);
			memcpy(partOfMsg, msgToParse, 10);
			int i = 0;
			for (i = atChar; i < atChar + 10; i++) {
				realMessage[i] = partOfMsg[i - atChar];
			}
			endProcessing = 1;
			atChar += 10;
		}
		else if(code[1] == 'c'){
			char partOfMsg[10];
			bzero(partOfMsg, 10);
			memcpy(partOfMsg, msgToParse, 10);
			int i = 0;
			for (i = atChar; i < atChar + 10; i++) {
				realMessage[i] = partOfMsg[i - atChar];
			}
			atChar += 10;
			endProcessing = 0;
		}

		if(atChar > sizeOfBuffer){
			break;
		}

		if(endProcessing == 1){
			realMessage[atChar] = '\0';
			break;
		}
	}
}

void parseMessage(int serverSocket){
	int atChar = 0;
	char realMessage[256];
	bzero(realMessage, 256);
	while(1){
		int endProcessing = 0;
		char buffer[13];
		char code[2];
		char size;
		bzero(buffer, 13);
		bzero(code, 2);
		read(serverSocket, buffer, 13);
		char* msgToParse = buffer;
		memcpy(code, msgToParse, 2);
		msgToParse += 2;
		size = msgToParse[0];
		msgToParse += 1;

		if(code[1] == 'a'){
			int realSize = size - 48;
			char partOfMsg[realSize];
			bzero(partOfMsg, realSize);
			memcpy(partOfMsg, msgToParse, realSize);
			int i = 0;
			for (i = atChar; i < atChar + realSize; i++) {
				realMessage[i] = partOfMsg[i - atChar];
			}
			endProcessing = 1;
			atChar += realSize;
		}
		else if(code[1] == 'e'){
			char partOfMsg[10];
			bzero(partOfMsg, 10);
			memcpy(partOfMsg, msgToParse, 10);
			int i = 0;
			for (i = atChar; i < atChar + 10; i++) {
				realMessage[i] = partOfMsg[i - atChar];
			}
			endProcessing = 1;
			atChar += 10;
		}
		else if(code[1] == 'c'){
			char partOfMsg[10];
			bzero(partOfMsg, 10);
			memcpy(partOfMsg, msgToParse, 10);
			int i = 0;
			for (i = atChar; i < atChar + 10; i++) {
				realMessage[i] = partOfMsg[i - atChar];
			}
			atChar += 10;
			endProcessing = 0;
		}

		if(endProcessing == 1){
			realMessage[atChar] = '\0';
			if(strcmp(realMessage, "enterUsr") == 0){
				puts("success");
			}
			puts(realMessage);
			atChar = 0;
		}
	}
}

void processTheMessageToServer(int clientSocket, char* message){
	while(1){
		int endSending = 0;
		if(strlen(message) > 10){
			char msgToSend[13];
			msgToSend[0] = '/';
			msgToSend[1] = 'c';
			msgToSend[2] = 48 + 0;
			int i = 0;
			for(i = 3; i < 13; i++){
				msgToSend[i] = message[i - 3];
			}

			write(clientSocket, msgToSend, 13);
			endSending = 0;
		}
		else if(strlen(message) < 10){
			int i = 0;
			char msgToSend[13];
			msgToSend[0] = '/';
			msgToSend[1] = 'a';
			msgToSend[2] = 48 + strlen(message);
			for(i = 3; i < strlen(message) + 3; i++){
				msgToSend[i] = message[i - 3];
			}

			for(i = strlen(message) + 3; i < 13; i++){
				msgToSend[i] = 'e';
			}

			write(clientSocket, msgToSend, 13);
			endSending = 1;
		}
		else if(strlen(message) == 10){
			char msgToSend[13];
			msgToSend[0] = '/';
			msgToSend[1] = 'e';
			msgToSend[2] = 48 + 0;
			int i = 0;
			for(i = 3; i < 13; i++){
				msgToSend[i] = message[i - 3];
			}

			write(clientSocket, msgToSend, 13);
			endSending = 1;
		}

		if(endSending == 0){
			message += 10;
		}
		else{
			break;
		}
	}
}

int processTheFileToServer(int serverSocket, char* filename){
	FILE *pf;
	char buffer[256];
	int fsize = 0;
	pf = fopen("/home/plt3ch/fileOneTest.txt", "rb");

	if (pf == NULL)
	{
	    printf("File not found!\n");
	    return 1;
	}
	else
	{
	  //  printf("Found file %s\n", filename);

	    fseek(pf, 0, SEEK_END);
	    fsize = ftell(pf);
	    rewind(pf);

	    printf("File contains %ld bytes!\n", fsize);
	    printf("Sending the file now");
	}

	while(1){
		bzero(buffer,sizeof(buffer));
		int bytes_read = fread(buffer, sizeof(char), sizeof(buffer), pf);
		if (bytes_read == 0) // We're done reading from the file
			break;

		if (bytes_read < 0)
		{
			error("ERROR reading from file");
		}
//			write(clientSocket, msgToSend, 13);

		 void *p = buffer;
		 while (bytes_read > 0)
		 {
			 int bytes_written = write(serverSocket, p, bytes_read);
			 puts("working");
			 if (bytes_written <= 0)
			 {
				 error("ERROR writing to socket\n");
			 }

			 bytes_read -= bytes_written;
			 p += bytes_written;
		 }
	}

	sleep(1);

	fclose(pf);
	return 0;
}


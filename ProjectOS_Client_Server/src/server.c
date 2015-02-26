/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "sqlLiteController.h"

void *connection_handler(void *socket_desc);

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int startServer(int portno)
{
     int sockfd, newsockfd, *new_sock, c;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (portno == 0) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");
     listen(sockfd,5);
     c = sizeof(struct sockaddr_in);
     while( (newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t*)&c)) ){
    	 pthread_t sniffer_thread;
    	 new_sock = malloc(sizeof(int));
    	 *new_sock = newsockfd;

    	 pthread_create( &sniffer_thread , 0 ,  connection_handler , (void*) new_sock);
    	 pthread_detach(sniffer_thread);
    	 //Now join the thread , so that we dont terminate before the thread
    	 //pthread_join( sniffer_thread , NULL);
    	 // puts("Handler assigned");
    	 // close(newsockfd);
     }

     return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
	//Get the socket descriptor
	int clientSocket = *(int*)socket_desc;
	char buffer[256];
	char username[256];
	short userFound = 0;
	short state = 0;

	//Send some messages to the client
	processTheMessageToClient(clientSocket, "Greetings! I am your connection handler! Its my duty to communicate with you!\nSelect option:");

	processTheMessageToClient(clientSocket, "1. Register\n2. Login");
	processTheMessageToClient(clientSocket, "RegLog");

	while(1){
		bzero(buffer, 256);
		receiveMessage(clientSocket, buffer, 256);
		if(userFound == 0){
			if(buffer[0] == '1'){
				registerForm(clientSocket);
			}else if(buffer[0] == '2'){
				bzero(username, 256);
				if(loginForm(clientSocket, username) == USER_EXIST){
					char message[256];
					bzero(message, 256);
					sprintf(message, "Hello, %s", username);
					processTheMessageToClient(clientSocket, message);

					userFound = 1;
				}
			}
		}

		if(userFound == 1){
			switch(state){
			case 0:
				processTheMessageToClient(clientSocket, "Do you want to send a file to your friends?(y/n)");
				processTheMessageToClient(clientSocket, "sndFQ");
				state = 1;
				break;
			case 1:
				if(strcmp(buffer, "y") == 0){
					state = 2;

//					processTheMessageToClient(clientSocket, "Choose file and the sending will start automatically");
					receiveFile(clientSocket);
				}
				break;
			}
		}

		usleep(500000);
	}

	free(socket_desc);
	return 0;
}

void registerForm(int clientSocket){
	char username[256];
	char password[256];

	usernameAndPasswordForm(clientSocket, username, password);
	createUserAcc(username, password);

	//realeasing the memory
//	free(code);
//	free(message);
//	free(buffer);
//	free(username);
//	free(password);
}

int loginForm(int clientSocket, char* username){
	char password[256];

	usernameAndPasswordForm(clientSocket, username, password);
	int accFound = checkForAccount(username, password);

	return accFound;
}

void usernameAndPasswordForm(int clientSocket, char* username, char* password){
	char buffer[256];

	processTheMessageToClient(clientSocket, "Username:");

	//Code: enterUser
	processTheMessageToClient(clientSocket, "enterUsr");

	bzero(buffer, 256);
	receiveMessage(clientSocket, buffer, 256);
	strcpy(username, buffer);

	//Password
	processTheMessageToClient(clientSocket, "Password:");

	//Code: enterPass
	processTheMessageToClient(clientSocket, "enterPass");

	bzero(buffer,256);
	receiveMessage(clientSocket, buffer, 256);
	strcpy(password, buffer);
}

void processTheMessageToClient(int clientSocket, char* message){
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

void receiveMessage(int serverSocket, char* realMessage, int size){
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

		if(atChar > size){
			break;
		}

		if(endProcessing == 1){
			realMessage[atChar] = '\0';
			break;
		}
	}
}

void receiveFile(int clientSocket){
	char buffer[256];
	bzero(buffer, 256);
	FILE *fp;
	int n;

	fp = fopen("fileOne.txt", "a+b");
	if (fp == NULL)
	{
		printf("File not found!\n");
		//return;
	}
	else
	{
		//printf("Found file %s\n", filename);
	}

	/* Time to Receive the File */

	do{
		bzero(buffer,256);
		n = read(clientSocket,buffer,256);
		if (n < 0) error("ERROR reading from socket");
		if(n > 0){
			fwrite(buffer, sizeof(char), strlen(buffer), fp);
		}

		puts("done");
//		if (n < 0) error("ERROR writing in file");

//		n = write(clientSocket,"I am getting your file...",25);
//		if (n < 0) error("ERROR writing to socket");
	} while (n>0);

	fclose(fp);
	return;
}

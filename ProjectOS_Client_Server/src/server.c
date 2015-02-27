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
#include <sys/stat.h>
#include "sqlLiteController.h"

#define FILES_FOLDER "userFiles/"
#define USERS_THREASHOLD 100

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t locks[USERS_THREASHOLD];
char *connectedUsers[USERS_THREASHOLD];
int connectedAndDisconnectedUsers[USERS_THREASHOLD];

void *connection_handler(void *socket_desc);
void *download_handler(void* currentUserIndex);

int startServer(int portno)
{
     int sockfd, newsockfd, *new_sock, c;
     struct sockaddr_in serv_addr, cli_addr;

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
              sizeof(serv_addr)) < 0){
              error("ERROR on binding");
     }

     listen(sockfd,5);

     c = sizeof(struct sockaddr_in);
     while( (newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t*)&c)) ){
    	 pthread_t sniffer_thread;
    	 new_sock = malloc(sizeof(int));
    	 *new_sock = newsockfd;

    	 pthread_create( &sniffer_thread , 0 ,  connection_handler , (void*) new_sock);
    	 pthread_detach(sniffer_thread);
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
	int userIndex;

	pthread_mutex_lock(&lock);
	int i;
	for (i = 0; i < USERS_THREASHOLD; i++) {
		if(connectedAndDisconnectedUsers[i] == 0){
			connectedAndDisconnectedUsers[i] = clientSocket;
			userIndex = i;
			pthread_mutex_t innerLock = PTHREAD_MUTEX_INITIALIZER;
			locks[userIndex] = innerLock;
			break;
		}
	}
	pthread_mutex_unlock(&lock);

	//Send some messages to the client
	processTheMessageToClient(clientSocket, "Greetings! I am your connection handler! Its my duty to communicate with you!\nSelect option:");
	processTheMessageToClient(clientSocket, "1. Register\n2. Login");
	processTheMessageToClient(clientSocket, "RegLog");

	while(1){
		bzero(buffer, 256);
		int n = receiveMessage(clientSocket, buffer, 256);
		if(n == 1){
			pthread_mutex_lock(&lock);
			puts(username);
			puts("End");
			connectedAndDisconnectedUsers[userIndex] = 0;
			connectedUsers[userIndex] = NULL;
			pthread_mutex_unlock(&lock);
			break;
		}

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
					connectedUsers[userIndex] = username;

					pthread_t fileDownloaderThread;
					int* newUserIndex = malloc(sizeof(int));
				    *newUserIndex = userIndex;
					pthread_create( &fileDownloaderThread , 0 ,  download_handler , (void*) newUserIndex);
					pthread_detach(fileDownloaderThread);
					sleep(1);
				}
			}
		}

		pthread_mutex_lock(&locks[userIndex]);
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
					bzero(buffer, 256);
					if(n!=1){
						n = receiveMessage(clientSocket, buffer, 256);
					}
					int fsize = atoi(buffer);

					bzero(buffer, 256);
					if(n!=1){
						n = receiveMessage(clientSocket, buffer, 256);
					}
					if(n!=1){
						n = receiveFile(clientSocket, fsize, buffer, userIndex);
					}
				}
				break;
			}
		}

		pthread_mutex_unlock(&locks[userIndex]);
		usleep(500000);
	}

	free(socket_desc);
	return 0;
}

void* download_handler(void* currentUserIndex){
	int userIndex = *(int*)currentUserIndex;
	char* username = connectedUsers[userIndex];

	while(1){
		pthread_mutex_lock(&lock);
		if(connectedAndDisconnectedUsers[userIndex] == 0){
			puts("end");
			break;
		}
		pthread_mutex_unlock(&lock);

		char* filenames[256];
		int size = getFilenamesToDownLoad(username, filenames);
		int i;
		for (i = 0; i < size; i++) {
			pthread_mutex_lock(&lock);
			if(connectedAndDisconnectedUsers[userIndex] == 0){
				puts("end");
				break;
			}
			pthread_mutex_unlock(&lock);
			pthread_mutex_lock(&locks[userIndex]);
			int clientSocket = connectedAndDisconnectedUsers[userIndex];
			processTheMessageToClient(clientSocket, "There are new files for yoy! Do you want to download them?(y/n)");
			processTheMessageToClient(clientSocket, "recFL");
			processTheFileToClient(clientSocket, filenames[i]);
			pthread_mutex_unlock(&locks[userIndex]);

			if(username != NULL){
				char temp[256];
				bzero(temp, 256);
				strcpy(temp, filenames[i]);
				deletePendingFilename(username, temp);
			}
		}

		sleep(15);
	}

	return 0;
}

void registerForm(int clientSocket){
	char username[256];
	char password[256];

	usernameAndPasswordForm(clientSocket, username, password);
	createUserAcc(username, password);
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

int processTheMessageToClient(int clientSocket, char* message){
	int n = 0;
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

			n = write(clientSocket, msgToSend, 13);
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

			n = write(clientSocket, msgToSend, 13);
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

			n = write(clientSocket, msgToSend, 13);
			endSending = 1;
		}

		if(endSending == 0){
			message += 10;
		}
		else{
			return 0;
		}

		if(n < 0){
			return 1;
		}
	}
}

int receiveMessage(int serverSocket, char* realMessage, int size){
	int atChar = 0;
	int n;
	do{
		int endProcessing = 0;
		char buffer[13];
		char code[2];
		char size;
		bzero(buffer, 13);
		bzero(code, 2);
		int n = read(serverSocket, buffer, 13);
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
			return 0;
		}
	}while(n>0);

	if(n <= 0){
		return 1;
	}

	return 0;
}

int receiveFile(int clientSocket, int fsize, char* filename, int userIndex){
	unsigned char buffer[256];
	bzero(buffer, 256);
	FILE *fp;
	int n;

	mkdir(FILES_FOLDER, S_IRWXU | S_IRWXG);

	char fullFilePath[256];
	bzero(fullFilePath, 256);
	strcpy(fullFilePath, FILES_FOLDER);
	strcat(fullFilePath, filename);

	fp = fopen(fullFilePath, "wb");
	if (fp == NULL)
	{
		printf("File not found!Rec\n");
		//return;
	}
	else
	{
		//printf("Found file %s\n", filename);
	}

	/* Time to Receive the File */
	while(fsize>0){
		bzero(buffer,256);
		n = read(clientSocket,buffer,256);
		if (n < 0) error("ERROR reading from socket");
		if(n > 0){
			fwrite(buffer, sizeof(char), strlen(buffer), fp);
		}

		fsize -= n;
	}

	fclose(fp);

	insertFileToDownLoad(connectedUsers[userIndex], fullFilePath);
	if(n<0){
		return 1;
	}
	return 0;
}

int processTheFileToClient(int clientSocket, char* filename){
	FILE *pf;
	unsigned char buffer[256];
	int fsize = 0;
	pf = fopen(filename, "rb");
	if (pf == NULL)
	{
		printf("File not foundProcess!\n");
		char* sizeInStrFormat = "0";
		processTheMessageToClient(clientSocket, sizeInStrFormat);
		return 1;
	}
	else
	{
		fseek(pf, 0, SEEK_END);
		fsize = ftell(pf);
		rewind(pf);

		printf("File contains %ld bytes!\n", fsize);
		printf("Sending the file now\n");
		char* filenamePos;
		filenamePos = strrchr(filename,'/');
		char sizeInStrFormat[15];
		sprintf(sizeInStrFormat, "%d", fsize);

		processTheMessageToClient(clientSocket, sizeInStrFormat);
		processTheMessageToClient(clientSocket, filenamePos + 1);
	}

	while(1){
		bzero(buffer,sizeof(buffer));
		int bytes_read = fread(buffer, sizeof(char), sizeof(buffer), pf);
		if (bytes_read == 0){ // We're done reading from the file
			break;
		}

		if (bytes_read < 0)
		{
			error("ERROR reading from file");
		}

		void *p = buffer;
		while (bytes_read > 0)
		{
			int bytes_written = write(clientSocket, p, bytes_read);
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

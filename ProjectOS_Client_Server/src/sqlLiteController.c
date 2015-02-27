#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include "sqlLiteController.h"

int checkForExistingUsername(char* name);
int insertUserAcc(char* name, char* password);

int createUserAcc(char* name, char* pass){
	int userExist = checkForExistingUsername(name);
	if(userExist != USER_EXIST){
		insertUserAcc(name, pass);
	}

	return 0;
}

//static int callback(void *data, int argc, char **argv, char **azColName){
//   int i;
//   fprintf(stderr, "%s: ", (const char*)data);
//   for(i=0; i<argc; i++){
//      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//   }
//   printf("\n");
//   return 0;
//}

int openDB(sqlite3 *db){
	int rc;
	/* Open database */


	return rc;
}

int checkForExistingUsername(char* name)
{
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   sqlite3_stmt *res;

   rc = sqlite3_open("/home/plt3ch/clientServerDB.db", &db);
   	if( rc ){
   		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
   		exit(0);
   	}else{
   		fprintf(stderr, "Opened database successfully\n");
   	}

//   char *sql2 = "Create table users(ID INTEGER PRIMARY KEY AUTOINCREMENT, NAME TEXT NOT NULL, PASSWORD TEXT NOT NULL);";

   char *sql = "SELECT NAME FROM users WHERE NAME = ?;";

   rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %d\n", rc);
      sqlite3_free(zErrMsg);
   }else{
	   sqlite3_bind_text(res, 1, name, strlen(name), SQLITE_STATIC);
   }

   int step = sqlite3_step(res);

   if (step == SQLITE_ROW) {
	   return USER_EXIST;
   }

   sqlite3_finalize(res);
   sqlite3_close(db);
   return 0;
}

int checkForAccount(char* name, char* password)
{
   int returnCode = 0;
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   sqlite3_stmt *res;

   rc = sqlite3_open("/home/plt3ch/clientServerDB.db", &db);
   	if( rc ){
   		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
   		exit(0);
   	}else{
   		fprintf(stderr, "Opened database successfully\n");
   	}

   char *sql = "SELECT NAME FROM users WHERE NAME = ? AND PASSWORD = ?;";

   rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %d\n", rc);
      sqlite3_free(zErrMsg);
   }else{
	   sqlite3_bind_text(res, 1, name, strlen(name), SQLITE_STATIC);
	   sqlite3_bind_text(res, 2, password, strlen(password), SQLITE_STATIC);
   }

   int step = sqlite3_step(res);

   if (step == SQLITE_ROW) {
	   returnCode = USER_EXIST;
   }

   sqlite3_finalize(res);
   sqlite3_close(db);
   return returnCode;
}

int insertUserAcc(char* name, char* password)
{
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   sqlite3_stmt *res;

   /* Open database */
   rc = sqlite3_open("/home/plt3ch/clientServerDB.db", &db);
   	if( rc ){
   		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
   		exit(0);
   	}else{
   		fprintf(stderr, "Opened database successfully\n");
   	}
   /* Create SQL statement */
   sql = "INSERT INTO USERS (NAME,PASSWORD) VALUES (?, ?);";

   rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
	   sqlite3_bind_text(res, 1, name, strlen(name), SQLITE_STATIC);
	   sqlite3_bind_text(res, 2, password, strlen(password), SQLITE_STATIC);
   }

   int step = sqlite3_step(res);

   sqlite3_finalize(res);
   sqlite3_close(db);
   return 0;
}

int insertFileToDownLoad(char* senderName, char* filename){
	int senderId = -1;

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	sqlite3_stmt *res;

	rc = sqlite3_open("/home/plt3ch/clientServerDB.db", &db);
	if( rc ){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		exit(0);
	}else{
		fprintf(stderr, "Opened database successfully\n");
	}

	char *sql = "SELECT ID FROM users WHERE NAME = ?;";

	rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %d\n", rc);
		sqlite3_free(zErrMsg);
	}else{
		sqlite3_bind_text(res, 1, senderName, strlen(senderName), SQLITE_STATIC);
	}

	int step = sqlite3_step(res);
	if (step == SQLITE_ROW) {
		senderId = sqlite3_column_int(res, 0);
	}

	sqlite3_finalize(res);

	if(senderId != -1){
		sqlite3_stmt *resFriends;
		char *sql = "SELECT FRIENDID FROM friends WHERE USERID = ?;";

		rc = sqlite3_prepare_v2(db, sql, -1, &resFriends, 0);

		if( rc != SQLITE_OK ){
			fprintf(stderr, "SQL error: %d\n", rc);
			sqlite3_free(zErrMsg);
		}else{
			sqlite3_bind_int(resFriends, 1, senderId);
		}

		int friendsIDs[256] = {-1};
		int index = 0;
		int stepForFriends = sqlite3_step(resFriends);
		while(stepForFriends == SQLITE_ROW){
			int userId = sqlite3_column_int(resFriends, 0);
			friendsIDs[index] = userId;
			index++;

			stepForFriends = sqlite3_step(resFriends);
		}

		sqlite3_finalize(resFriends);

		int i = 0;
		for (i = 0; i < 256; i++) {
			if(friendsIDs[i] == -1 || friendsIDs[i] == 0){
				continue;
			}

			int userId = friendsIDs[i];
			sqlite3_stmt *resPendingFiles;
			char *sql = "INSERT INTO pendingFiles (USERID,SENDERID,FILENAME) VALUES (?, ?, ?);";
			rc = sqlite3_prepare_v2(db, sql, -1, &resPendingFiles, 0);

			if( rc != SQLITE_OK ){
				fprintf(stderr, "SQL error: %d\n", rc);
				sqlite3_free(zErrMsg);
			}else{
				sqlite3_bind_int(resPendingFiles, 1, userId);
				sqlite3_bind_int(resPendingFiles, 2, senderId);
				sqlite3_bind_text(resPendingFiles, 3, filename, strlen(filename), SQLITE_STATIC);
			}

			sqlite3_step(resPendingFiles);
			sqlite3_finalize(resPendingFiles);
		}
	}

	return 0;
}

int getFilenamesToDownLoad(char* username, char** filenames){
	int userId = -1;
	int filenamesSize = 0;

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	sqlite3_stmt *res;

	rc = sqlite3_open("/home/plt3ch/clientServerDB.db", &db);
	if( rc ){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		exit(0);
	}else{
		fprintf(stderr, "Opened database successfully\n");
	}

	char *sql = "SELECT ID FROM users WHERE NAME = ?;";

	rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %d\n", rc);
		sqlite3_free(zErrMsg);
	}else{
		sqlite3_bind_text(res, 1, username, strlen(username), SQLITE_STATIC);
	}

	int step = sqlite3_step(res);
	if (step == SQLITE_ROW) {
		userId = sqlite3_column_int(res, 0);
	}

	sqlite3_finalize(res);

	if(userId != -1){
		sqlite3_stmt *resPendingFiles;
		char *sql = "SELECT FILENAME FROM pendingFiles WHERE USERID = ?;";

		rc = sqlite3_prepare_v2(db, sql, -1, &resPendingFiles, 0);

		if( rc != SQLITE_OK ){
			fprintf(stderr, "SQL error: %d\n", rc);
			sqlite3_free(zErrMsg);
		}else{
			sqlite3_bind_int(resPendingFiles, 1, userId);
		}

		int index = 0;
		int stepForFriends = sqlite3_step(resPendingFiles);
		while(stepForFriends == SQLITE_ROW){
			char* filename = sqlite3_column_text(resPendingFiles, 0);
			char* tempFilename = calloc(256*sizeof(char),1);
//			bzero(tempFilename, 256);
			strcpy(tempFilename, filename);
			filenames[index] = tempFilename;
			index++;
			stepForFriends = sqlite3_step(resPendingFiles);
		}

		filenamesSize = index;
		sqlite3_finalize(resPendingFiles);

	}

	return filenamesSize;
}

void deletePendingFilename(char* username, char* filenameToDelete){
	int userId = -1;
	int filenamesSize = 0;

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	sqlite3_stmt *res;

	rc = sqlite3_open("/home/plt3ch/clientServerDB.db", &db);
	if( rc ){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		exit(0);
	}else{
		fprintf(stderr, "Opened database successfully\n");
	}

	char *sql = "SELECT ID FROM users WHERE NAME = ?;";

	rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %d\n", rc);
		sqlite3_free(zErrMsg);
	}else{
		sqlite3_bind_text(res, 1, username, strlen(username), SQLITE_STATIC);
	}

	int step = sqlite3_step(res);
	if (step == SQLITE_ROW) {
		userId = sqlite3_column_int(res, 0);
	}

	sqlite3_finalize(res);

	if(userId != -1){
		sqlite3_stmt *resPendingFiles;
		char *sql = "DELETE FROM pendingFiles WHERE USERID = ? AND FILENAME = ?;";

		rc = sqlite3_prepare_v2(db, sql, -1, &resPendingFiles, 0);

		if( rc != SQLITE_OK ){
			fprintf(stderr, "SQL error: %d\n", rc);
			sqlite3_free(zErrMsg);
		}else{
			sqlite3_bind_int(resPendingFiles, 1, userId);
			sqlite3_bind_text(resPendingFiles, 2, filenameToDelete, strlen(filenameToDelete), SQLITE_STATIC);
		}

		int stepForFriends = sqlite3_step(resPendingFiles);
		sqlite3_finalize(resPendingFiles);
	}

	return;
}

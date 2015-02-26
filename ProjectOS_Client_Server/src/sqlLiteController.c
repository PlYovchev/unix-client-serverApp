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
	   return USER_EXIST;
   }

   sqlite3_finalize(res);
   sqlite3_close(db);
   return 0;
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

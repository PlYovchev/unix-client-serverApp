/*
 * sqlLiteController.h
 *
 *  Created on: Feb 20, 2015
 *      Author: plt3ch
 */

#ifndef SQLLITECONTROLLER_H_
#define SQLLITECONTROLLER_H_
#define USER_EXIST 3

int createUserAcc(char* name, char* pass);
int checkForAccount(char* name, char* password);

#endif /* SQLLITECONTROLLER_H_ */

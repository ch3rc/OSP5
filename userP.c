//==============================================================
//Date:		April 2,2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 5
//File:		userP.c
//==============================================================

#include "share.h"

int main(int argc, char *argv[])
{

int number = atoi(argv[0]);

fprintf(stderr,"made it into child = %d\n", getpid());

fprintf(stderr,"obtaining memory now\n");

Clock* clockPtr = (Clock*)getMem(clockKey, clockSize, &clockID);
Msg* msgPtr = (Msg*)getMem(msgKey, msgSize, &msgID);
ReqCheck* reqPtr = (ReqCheck*)getMem(reqKey, reqSize, &reqID);
sem_t* semPtr = getSem(semKey, semSize, semID);


fprintf(stderr, "child  has successfully obtained memory\n");


fprintf(stderr, "child now exiting\n");

while(msgPtr->state != 1)
	;

detachMem();
exit(12);

}

//==================================================================
//Date: 	April 2,2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 5
//File:		oss.c
//==================================================================

#include "share.h"

int main(int argc, char *argv[])
{
	pid_t childPid;
	int exitPid = 0;
	int status = 0;

	Clock* clockPtr = (Clock*)getMem(clockKey, clockSize, &clockID);
	Msg* msgPtr = (Msg*)getMem(msgKey, msgSize, &msgID);
	ReqCheck* reqPtr = (ReqCheck*)getMem(reqKey, reqSize, &reqID);
	sem_t* semPtr = getSem(semKey, semSize, &semID);

	initClock(clockPtr);

	childPid = fork();

	if(childPid < 0)
	{
		perror("ERROR: OSS: fork\n");
		cleanAll();
		exit(1);
	}
	
	if(childPid == 0)
	{
		char str[20];
		snprintf(str, sizeof(str), "%d", 1);
		execlp("./userP", str, NULL);
	}
	msgPtr->state = 0;

	fprintf(stderr, "OSS: test the rest\n");
	sleep(1);

	msgPtr->state = 1;

	if((exitPid = waitpid((pid_t)-1, &status, NULL)) > 0)
	{
		if(WIFEXITED(status))
		{
			if(WEXITSTATUS(status) == 12)
			{
				printf("OSS: child process has ended\n");
			}
		}
	}

	printf("OSS: finished and ending process\n");
	cleanAll();
	return 0;
}

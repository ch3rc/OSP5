//==================================================================
//Date: 	April 2,2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 5
//File:		oss.c
//==================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/msg.h>
#include <semaphore.h>
#include "share.h"

typedef struct{

	long mtype;
	char msg[100];
}Msg;


int main(int argc, char *argv[])
{
	/* shared mem pointers clock, req, sem */

	srand(time(NULL));
	
	pid_t childPid;

	initMem();

	message();

	Msg msg;

	allAtOnce();

	initClock(clock);
	
	//printDescriptor(req);
	
	//deadLock(req);
	
	Clock newProc;
	newProc.seconds = 0;
	newProc.nano = (rand() % 500000000) + 1;

	Clock deadcheck;
	deadcheck.seconds = 1;
	deadcheck.nano = 0;

		

	while(1)
	{
		int i = 0;
		if(compare(&clock, &newProc) == 0 && launched < MAX_PROCESSES)
		{
			childPid = fork();

			if(childpid < 0)
			{
				perror("ERROR: OSS: fork\n");
				exit(1);
			}
			
			if(childPid == 0)
			{
				execv("./userP", NULL);
			}
			
			launched++;
			
			int pos = findSpot();
		
			req->pid[pos] = getpid();
			
			//increase clock for next process launch
			newProc.seconds += clock->seconds;
			newProc.nano += clock->nano;
			unsigned int spawn = (rand() % 500000000) + 1;
			tickClock(&newProc, spawn);

		}

		if(compare(&clock, &deadcheck) == 0)
		{
			deadLock(req);
			deadcheck.seconds = clock->seconds;
			deadcheck.nano = clock->nano;
			unsigned int newDead = 1000000000;
			tickClock(&deadcheck, newDead);
		}

		while(i < MAX_PROCESSES)
		{

			if(msgrcv(toOSS, &msg, sizeof(msg) req->pid[i], IPC_NOWAIT) > 0)
			{
				if(strcmp(msg.msg, "TERMINATE") == 0)
				{
					offThePid(req, req->pid[i]);
				}
			}

			i++;
		}

		//TODO: msgrcv for terminate
		//TODO: msgrcv for request
		//TODO: msgrcv for release
		//TODO: add time to clock with sems

		
	}

		

				
	
	

	printf("OSS: finished and ending process\n");
	clearMessage();
	cleanAll();
	return 0;
}



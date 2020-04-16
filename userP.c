//==============================================================
//Date:		April 2,2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 5
//File:		userP.c
//==============================================================
#include <stdio.h>
#include <stdlib.h>
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
	fprintf(stderr, "YOU MADE IT IN\n");

	/*shared mem pointers clock, req, sem */
	signal(SIGTERM, &destroy);
	signal(SIGINT, &destroy);
	
	srand(time(0) ^ (getpid() << 16));

	int p = atoi(argv[0]);

	fprintf(stderr, "child position is %d\n", p);

	initMem();

	fprintf(stderr, "child got memory\n");

	message();

	Msg msg;
	

	//initial time to check for termination of 1 second
	Clock termtime;
	termtime.nano = clock->nano;
	termtime.seconds = clock->seconds + 1;

	//fprintf(stderr, "my pid = %d\n", req->pid[p]);	

	
	while(1)
	{
		msgrcv(toUSR, &msg, sizeof(msg), req->pid[p], IPC_NOWAIT);
		
			
		
		if(strcmp(msg.msg, "DEAD") == 0)
		{
			msg.mtype = req->pid[p];
			strcpy(msg.msg, "DEADLOCKED");
			msgsnd(toOSS, &msg, sizeof(msg), IPC_NOWAIT);
			detachMem();
			exit(12);
		}


		//terminate
		if(compare(clock, &termtime) == 1)
		{
			if((rand() % 100) <= 20)
			{
				//fprintf(stderr, "Terminating\n");
				msg.mtype = req->pid[p];
				strcpy(msg.msg, "TERMINATE");
				msgsnd(toOSS, &msg, sizeof(msg), IPC_NOWAIT);
				detachMem();
				exit(12);
			}
			termtime.nano = clock->nano;
			termtime.seconds = clock->seconds;
			unsigned int bound = (rand() % 2500000);
			tickClock(&termtime, bound);
		}
		//release
		if((rand() % 100) >= 80)
		{

			int itemToRequest = (rand() % 20);
			req->request[p][itemToRequest] = 1;

			msg.mtype = req->pid[p];
			strcpy(msg.msg, "REQUEST");
			msgsnd(toOSS, &msg, sizeof(msg), IPC_NOWAIT);

			while(1)
			{
				if(msgrcv(toUSR, &msg, sizeof(msg), req->pid[p], IPC_NOWAIT) > 0)
				{	
					if(strcmp(msg.msg, "ACCEPTED") == 0)
					{
						break;
					}

					if(strcmp(msg.msg, "DEAD") == 0)
					{
						msg.mtype = req->pid[p];
						strcpy(msg.msg, "DEADLOCKED");
						msgsnd(toOSS, &msg, sizeof(msg), IPC_NOWAIT);
						detachMem();
						exit(12);
					}
				}
			
			}

		}
		else
		{
			int itemToRelease = (rand() % 20);
			if(req->curAlloc[p][itemToRelease] != 0)
			{
				req->release[p][itemToRelease] = 1;
				msg.mtype = req->pid[p];
				strcpy(msg.msg, "RELEASE");
				msgsnd(toOSS, &msg, sizeof(msg), IPC_NOWAIT);
			}
	
		}

		

		sem_wait(sem);
		tickClock(clock, 10000);
		sem_post(sem);	

	}
	

}

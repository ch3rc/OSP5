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

int main(int argc, char *argv[])
{

	/*shared mem pointers clock, req, sem */
	signal(SIGTERM, &destroy);
	signal(SIGINT, &destroy);
	
	srand(time(0) ^ (getpid() << 16));

	int p = atoi(argv[0]);

	int pid = getpid();

	initMem();

	message();

	Msg msg;
	
	req->rpid[p] = pid;

	//initial time to check for termination of 1 second
	Clock termtime;
	termtime.nano = clock->nano;
	termtime.seconds = clock->seconds + 1;	
	
	int test, block;
	
	while(1)
	{
		test = 0;
		block = 0;	
		msgrcv(toUSR, &msg, sizeof(msg), req->rpid[p], IPC_NOWAIT);
		
			
		
		if(strcmp(msg.msg, "DEAD") == 0)
		{
			msg.mtype = req->rpid[p];
			strcpy(msg.msg, "DEADLOCK");
			msgsnd(toOSS, &msg, sizeof(msg), 0);
			detachMem();
			exit(12);
		}


		//terminate
		if(compare(clock, &termtime) == 1)
		{
			if((rand() % 100) <= 20)
			{
				msg.mtype = req->rpid[p];
				strcpy(msg.msg, "TERMINATE");
				msgsnd(toOSS, &msg, sizeof(msg), 0);
				detachMem();
				exit(12);
			}
			termtime.nano = clock->nano;
			termtime.seconds = clock->seconds;
			unsigned int bound = (rand() % 250000000);
			tickClock(&termtime, bound);
		}
		//release
		if((rand() % 100) >= 85)
		{

			int itemToRequest = (rand() % 20);
			req->request[p][itemToRequest] = (rand() % 10) + 1;
			//fprintf(stderr, "sending a request %d\n", req->pid[p]);
			msg.mtype = req->rpid[p];
			strcpy(msg.msg, "REQUEST");
			msgsnd(toOSS, &msg, sizeof(msg), 0);
			test = 1;

			while(1)
			{
				if(test == 1 && block != 1)
				{
					//fprintf(stderr, "\nstuck in loop %d\n", req->pid[p]);
					block = 1;
				}
				
				if(msgrcv(toUSR, &msg, sizeof(msg), req->rpid[p], 0) > 0)
				{
	
					if(strcmp(msg.msg, "ACCEPTED") == 0)
					{
						//fprintf(stderr, "\n%d has been accepted\n", req->pid[p]);
						break;
					}

					if(strcmp(msg.msg, "DEAD") == 0)
					{
						msg.mtype = req->rpid[p];
						strcpy(msg.msg, "DEADLOCK");
						msgsnd(toOSS, &msg, sizeof(msg), 0);
						//fprintf(stderr, "\nmade it into DEAD\n");
						//raise(SIGTERM);
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
				msg.mtype = req->rpid[p];
				strcpy(msg.msg, "RELEASE");
				msgsnd(toOSS, &msg, sizeof(msg), 0);
			}
	
		}
		

		

		sem_wait(sem);
		tickClock(clock, (rand() % 250000000));
		sem_post(sem);	

	}
	

}

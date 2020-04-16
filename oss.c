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

int unaturalDeath;
int naturealDeath;
int launched;
int death;

int main(int argc, char *argv[])
{
	/* shared mem pointers clock, req, sem */

	srand(time(NULL));
	
	pid_t childPid;
	int exitpid = 0;

	int status = 0;
	
	initMem();

	message();

	Msg msg;

	allAtOnce();

	initClock(clock);
	
	signal(SIGINT, &killAll);
	signal(SIGALRM, &timesUp);
	alarm(3);
	
	Clock newProc;
	newProc.seconds = 0;
	newProc.nano = (rand() % 500000000) + 1;

	Clock deadcheck;
	deadcheck.seconds = 1;
	deadcheck.nano = 0;

	struct Queue *queue = NULL;
	
	int i = 0;

	while(death < 20)
	{
		
		if(compare(clock, &newProc) == 0 && launched < MAX_PROCESSES)
		{
			int pos = findSpot();

			if(pos > -1)
			{
				childPid = fork();

				if(childPid < 0)
				{
					perror("ERROR: OSS: fork\n");
					endProcesses();
					exit(1);
				}
			
				if(childPid == 0)
				{
					char id[20];
					snprintf(id, sizeof(id), "%d", pos);
					execl("./userP", id, NULL);
				}
			
				launched++;
				
				req->pid[pos] = pos;
				
				fprintf(stderr, "launching child = %d\n", req->pid[pos]);	
			
				//increase clock for next process launch
				newProc.seconds += clock->seconds;
				newProc.nano += clock->nano;
				unsigned int spawn = (rand() % 500000000) + 1;
				tickClock(&newProc, spawn);
			}

		}

		if(compare(clock, &deadcheck) == 1)
		{
			deadLock(req, queue);
			deadcheck.seconds = clock->seconds;
			deadcheck.nano = clock->nano;
			unsigned int newDead = 1000000000;
			tickClock(&deadcheck, newDead);
		}

		
	
		if(msgrcv(toOSS, &msg, sizeof(msg), req->pid[i], IPC_NOWAIT) > 0)
		{
			if(strcmp(msg.msg, "TERMINATE") == 0)
			{
				offThePid(req, req->pid[i]);
				if(exitpid = waitpid(req->pid[i], &status, NULL) > 0)
				{
					if(WIFEXITED(status))
					{
						printf("a process has terminated\n");
						launched--;
						death++;
						naturalDeath++;
					}
				}
			}

			if(strcmp(msg.msg, "RELEASE") == 0)
			{
				releaseResource(req, req->pid[i]);
			}

			if(strcmp(msg.msg, "REQUEST") == 0)
			{
				checkRequest(queue, req, req->pid[i]);
			}

			if(strcmp(msg.msg, "DEADLOCK") == 0)
			{
				fprintf(stderr, "its making it in here at least\n");
				if(exitpid = waitpid(req->pid[i], &status, NULL) > 0);
				{
					fprintf(stderr,"casualty of deadlock\n");
					launched--;
					death++;
					unaturalDeath++;
				}
			}
		}

		if(search(queue, req->pid[i]) == 1)
		{
			checkAgain(queue, req, req->pid[i]);
		}

		i++;
		
		sem_wait(sem);
		tickClock(clock, 10000);
		sem_post(sem);

			
		//TODO: msgrcv for request

		if(death == 20)
		{
			break;
		}

		if(i == 17)
		{
			i = 0;
		}
	
	}

		

				
	//endProcesses();	
	

	printf("OSS: finished and ending process\n");
	clearMessage();
	cleanAll();
	return 0;
}



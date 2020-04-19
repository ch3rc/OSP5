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

/*typedef struct{

	long mtype;
	char msg[100];
}Msg;*/

int unaturalDeath = 0;
int naturalDeath = 0;
int written = 0;
int launched = 0;
int death = 0;
int granted = 0;
int availMatrix[MAX_RESOURCES];
int resMatrix[MAX_RESOURCES];
int reqMatrix[MAX_PROCESSES][MAX_RESOURCES];
int allocMatrix[MAX_PROCESSES][MAX_RESOURCES];
int granted;
int verbose;
char *filename;
FILE *fp;

int main(int argc, char *argv[])
{
	/* shared mem pointers clock, req, sem */

	srand(time(NULL));
	
	initOpts();

	setOpts(argc, argv);

	fp = fopen(filename, "w");

	if(fp == NULL)
	{
		perror("ERROR: OSS: FILE\n");
		exit(1);
	}

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
	alarm(6);
	
	Clock newProc;
	newProc.seconds = 0;
	newProc.nano = (rand() % 500000000) + 1;

	Clock deadcheck;
	deadcheck.seconds = 1;
	deadcheck.nano = 0;

	struct Queue *queue = NULL;
	
	int i = 0;
	
	int printed = 0;

	int temp = 0;
	
	

	while(death < 200)
	{
			
		if(compare(clock, &newProc) == 0 && launched < MAX_PROCESSES && death < 200)
		{
			int pos = findSpot();

			if(pos == -1)
			{
				continue;
			}
			else
			{
				childPid = fork();

				if(childPid < 0)
				{
					perror("ERROR: OSS: fork\n");
					//endProcesses();
					//exit(1);
				}
			
				if(childPid == 0)
				{
					char id[20];
					snprintf(id, sizeof(id), "%d", pos);
					execl("./userP", id, NULL);
				}
			
				launched++;
				
				req->pid[pos] = pos;
				
				//fprintf(stderr, "launching child = %d\n", req->pid[pos]);	
			
				//increase clock for next process launch
				newProc.seconds += clock->seconds;
				newProc.nano += clock->nano;
				unsigned int spawn = (rand() % 500000000) + 1;
				tickClock(&newProc, spawn);
			}

		}

		if(compare(clock, &deadcheck) == 1 && launched > 0)
		{
			deadLock(req);
			deadcheck.seconds = clock->seconds;
			deadcheck.nano = clock->nano;
			unsigned int newDead = 1000000000;
			tickClock(&deadcheck, newDead);
		}

		
			
		if(msgrcv(toOSS, &msg, sizeof(msg), req->rpid[i], IPC_NOWAIT) > 0)
		{
			
			if(strcmp(msg.msg, "TERMINATE") == 0)
			{
				offThePid(req, req->pid[i]);
				if(exitpid = waitpid(req->pid[i], &status, NULL) > 0)
				{
					if(WIFEXITED(status))
					{
						//printf("a process has terminated P%d dead = %d\n",i, death + 1);
						launched--;
						death++;
						naturalDeath++;
					}
				}
			}

			if(strcmp(msg.msg, "DEADLOCK") == 0)
			{
				if(exitpid == waitpid(req->pid[i], &status, NULL) > 0)
				{
					if(WIFEXITED(status))
					{
						//fprintf(stderr, "\nDEADLOCK PROCESS P%d HAS TERMINATED, DEAD = %d\n", i, death + 1);
						updatePid(i);
						launched--;
						death++;
						unaturalDeath++;
					}
				}
			}

			if(strcmp(msg.msg, "RELEASE") == 0)
			{
				releaseResource(req, req->pid[i]);
			}

			if(strcmp(msg.msg, "REQUEST") == 0)
			{
				checkRequest(req, req->pid[i]);
			}
			
		}
		
		
		i++;
		if(granted % 20 == 0 && granted != 0 && printed == 0)
		{
			temp = granted;
			printMatrix(allocMatrix);
			printed = 1;
		}	

		if((temp + 20) == granted)
		{
			printMatrix(allocMatrix);
			temp += granted;
		}
		
		if(search(queue, req->pid[i]) > -1)
		{	
			checkAgain(req, req->pid[i]);
		}

		//display(queue);
		
		sem_wait(sem);
		tickClock(clock, 10000);
		sem_post(sem);

			
		//TODO: msgrcv for request

		if(death == 200)
		{
			//fprintf(stderr, "\nNeed to know if it made it into break point\n");
			break;
		}

		if(i == 17)
			i = 0;
	
	}
	
	fclose(fp);
	
	printf("deadlock kills = %d\n", unaturalDeath);
	printf("terminations = %d\n", naturalDeath);
		

				
	while(exitpid = wait(&status) > 0);
	
	//fprintf(stderr, "OSS: finished and ending process\n");
	clearMessage();
	cleanAll();
	return 0;
}

//======================================================================================================
//Getopt stuff
//======================================================================================================
void initOpts()
{
	verbose = 0;
	filename = NULL;
}

void setOpts(int argc, char *argv[])
{
	int c;
	while((c = getopt(argc, argv, "v")) != -1)
	{
		switch(c)
		{
			case 'v':
				verbose = 1;
				break;
			case '?':
				printf("unknown option %s\n",optarg);
				exit(1);
				break;
		}
	}

	if(optind < argc)
	{
		filename = argv[optind];
	}
}


//=======================================================================================================
//All resource functions that have a queue. This is a test to see if this works
//=======================================================================================================

//Accept or deny request for resource
void checkRequest(Descriptor *req, int pos)
{
	Msg msg;
	int column, accept = 0;
	for(column = 0; column < MAX_RESOURCES; column++)
	{
		if(req->request[pos][column] != 0)
		{
			if(written < 100000 && verbose == 1)
			{
				fprintf(fp, "OSS has detected P%d requesting R%d at time %d:%d\n",
					pos, column, clock->seconds, clock->nano);
				written++;
			}
			
			int maxAlloc = req->curAlloc[pos][column] + req->request[pos][column];
			if(maxAlloc <= availMatrix[column] && availMatrix[column] != 0)
			{
				if(written < 100000 && verbose == 1)
				{
					fprintf(fp, "OSS granting P%d request R%d at time %d:%d\n",
						pos, column, clock->seconds, clock->nano);
					written++;
				}

				req->curAlloc[pos][column] += req->request[pos][column];
				allocMatrix[pos][column] = req->curAlloc[pos][column];
				availMatrix[column] -= req->request[pos][column];
				updateRequest(req, reqMatrix);
				req->request[pos][column] = 0;
				granted++;
				accept = 1;
			}
			else
			{
				if(written < 100000 && verbose == 1)
				{
					fprintf(fp, "OSS denying P%d request R%d at time %d:%d\n",
						pos, column, clock->seconds, clock->nano);
					written++;
				}
				
				insert(&queue, req->pid[pos]);
			}
		}
	}

	if(accept)
	{
		msg.mtype = req->rpid[pos];
		strcpy(msg.msg, "ACCEPTED");
		msgsnd(toUSR, &msg, sizeof(msg), IPC_NOWAIT);
	}
}


//check to see if resources are now available
void checkAgain(Descriptor *req, int pos)
{
	Msg msg;
	int column;
	for(column = 0; column < MAX_RESOURCES; column++)
	{
		if(req->request[pos][column] != 0)
		{
			int maxAlloc = req->curAlloc[pos][column] + req->request[pos][column];
			if(maxAlloc <= availMatrix[column] && availMatrix[column] != 0)
			{
				if(written < 100000 && verbose == 1)
				{
					fprintf(fp, "OSS checking again and granting P%d request R%d at time %d:%d\n",
						pos, column, clock->seconds, clock->nano);
					written++;
				}

				req->curAlloc[pos][column] += req->request[pos][column];
				allocMatrix[pos][column] = req->curAlloc[pos][column];
				updateRequest(req, reqMatrix);
				availMatrix[column] -= req->request[pos][column];
				req->request[pos][column] = 0;
				deleteQ(&queue, req->pid[pos]);
				granted++;
				msg.mtype = req->rpid[pos];
				strcpy(msg.msg, "ACCEPTED");
				msgsnd(toUSR, &msg, sizeof(msg), 0);
			}
			else
			{
				return;
			}
		}
	}
}

//deadlock check
void deadLock(Descriptor *req)
{
	Msg msg;
	
	int i, j, k;
	
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		if(search(queue, req->pid[i]) > -1)
		{
			k++;
		}
	}

	if(k >= 10)
	{
		if(written < 100000)
		{
			fprintf(fp, "OSS running deadlock detection at time %d:%d\n",
				clock->seconds, clock->nano);
			fprintf(fp, "Processes deadlocked: ");
			written += 2;
		}

		for(i = 0; i < MAX_PROCESSES; i++)
		{
			if(search(queue, req->pid[i]) > -1)
			{
				if(written < 100000)
				{
					fprintf(fp, "P%d ", i);
					written++;
				}
			}
		}

		if(written < 100000)
		{
			fprintf(fp, "\nAttempting to reslove deadlock\n");
			written++;
		}

		for(i = 0; i < MAX_PROCESSES; i++)
		{
			if(search(queue, req->pid[i]) > -1)
			{
				if(written < 100000)
				{
					fprintf(fp, "\nkilling process P%d\n", i);
					fprintf(fp, "Releasing resources:");
					written += 2;
				}
				for(j = 0; j < MAX_PROCESSES; j++)
				{
					if(written < 100000)
					{
						fprintf(fp, " R%d", j);
						written++;
					}
					availMatrix[i] += req->curAlloc[i][j];
				}
				
				deleteQ(&queue, req->pid[i]);
				msg.mtype = req->rpid[i];
				strcpy(msg.msg, "DEAD");
				msgsnd(toUSR, &msg, sizeof(msg), IPC_NOWAIT);
			}
		}
	}
	else
	{
		return;
	}
}

//=====================================================================
//Date:		April 10,2020
//Author:	Cody Hawkins
//Class:	CS4760
//Program:	Assignment 5
//File:		resources.c
//=====================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <error.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/msg.h>
#include <semaphore.h>
#include "share.h"
#include "queue.h"

int availMatrix[MAX_RESOURCES];
int resMatrix[MAX_RESOURCES];
int reqMatrix[MAX_PROCESSES][MAX_RESOURCES];
int allocMatrix[MAX_PROCESSES][MAX_RESOURCES];

int unaturalDeath = 0;
int naturalDeath = 0;
int written = 0;
int launched = 0;
int death = 0;

Queue *queue = creatQueue(MAX_PROCESSES);

/*shared memory pointers clock(timer), req(resources), sem(semaphore)*/
typedef struct{
	
	long mtype;
	char msg[100];
}Msg;

//move clock and give it a little leeway with
//unsigned integer. 4.3 x 10^9
void tickClock(Clock *clock, unsigned int nanos)
{
	unsigned int nano = clock->nano + nanos;
	
	while(nano >= 1000000000)
	{
		nano -= 1000000000;
		clock->seconds++;
	}
	
	clock->nano = nano;
}

int compare(Clock *time, Clock* newtime)
{
	long time1 = (long)time->seconds * 1000000000 + (long)time->nano;
	long time2 = (long)newtime->seconds * 1000000000 + (long)newtime->nano;

	if(time1 > time2)
		return 1;
	else
		return 0;
}

//initialize matrix with all zeros
void initMatrix(int arr[MAX_PROCESSES][MAX_RESOURCES])
{
	int i, j;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		for(j = 0; j < MAX_RESOURCES; j++)
		{
			arr[i][j] = 0;
		}
	}
}

//initialize vector with all zeros
void initVec(int arr[MAX_RESOURCES])
{
	int i;
	for(i = 0; i < MAX_RESOURCES; i++)
	{
		arr[i] = 0;
	}
}

//get position of filled descriptor 
int getPos(Descriptor* req, int pid)
{
	int i;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		if(req->pid[i] == pid)
		{
			return i;
		}
	}
	return -1;
}
//find open spot in empty descriptor
int findSpot()
{
	int i;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		if(req->pid[i] == 0)
		{
			return i;
		}
	}
	return -1;
}		
//initialize descriptor with allocation 
//instances (1 - 10) inclusive
void initDescriptor(Descriptor* req)
{

	int i, j;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		req->pid[i] = 0;

		for(j = 0; j < MAX_RESOURCES; j++)
		{
			req->maxRes[i][j] = 0;
			req->curAlloc[i][j] = (rand() % 10) + 1;
			req->release[i][j] = 0;
			req->request[i][j] = 0;
		}
		//zero is not shared, 1 is shared
		req->shared[i] = (((rand() % 100) + 1) < 20) ? 1 : 0;
	}
}


//fill request and allocation static tables
//also fill max requests for descriptors
//allocation + request table values.
void fillTables(Descriptor* req, int alloc[MAX_PROCESSES][MAX_RESOURCES],int avail[MAX_RESOURCES])
{
	int i, j;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		for(j = 0; j < MAX_RESOURCES; j++)
		{
			avail[i] += req->curAlloc[i][j];
			alloc[i][j] = req->curAlloc[i][j];
		}
	}
}

void updateRequest(Descriptor *req, int array[MAX_PROCESSES][MAX_RESOURCES])
{
	int i, j;
	
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		for(j = 0; j < MAX_RESOURCES; j++)
		{
			array[i][j] += req->request[i][j];
		}
	}
}

//terminate pid and release allocations back
//to available.
void offThePid(Descriptor *req, int pid)
{
	int i = getPos(req, pid);
	if(written < 100000)
	{
		fprintf(stderr, "OSS Terminating P%d at time %d:%d\n",i, clock->seconds, clock->nano);
		written++;
	}

	naturalDeath++;

	int j, k;
	for(j = 0; j < MAX_PROCESSES; j++)
	{
		if(j == i)
		{
			req->shared[i] = 0;
			for(k = 0; k < MAX_RESOURCES; k++)
			{
				req->maxRes[j][k] = 0;
				resMatrix[k] += req->curAlloc[j][k];
				req->curAlloc[j][k] = 0;
				req->release[j][k] = 0;
				req->request[j][k] = 0;
				reqMatrix[j][k] = 0;
			}
			kill(req->pid[j], SIGTERM);
			req->pid[j] = 0;
			launched--;
			death++;
		}
		break;
	}
	updatePid(i);
}

//after terminating a pid, update allocations
//and request table
void updatePid(int pos)
{

	int i, j;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		if(req->pid[i] == pos)
		{
			for(j = 0; j < MAX_RESOURCES; j++)
			{
				req->curAlloc[i][j] = (rand()% 10) + 1;
				reqMatrix[i][j] = req->curAlloc[i][j];
			}
		}
		req->shared[i] = (((rand() % 100) + 1) < 20) ? 1 : 0;
	}
}

void releaseResource(Descriptor *req, int pid)
{
	if(written < 100000)
	{
		fprintf(stderr, "OSS has acknowledged process P%d requesting to release resources at time %d:%d\n",
			pid, clock->seconds, clock->nano);
		written++;
	}

	
	int i, j;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		if(req->pid[i] == pid)
		{
			for(j = 0; j < MAX_RESOURCES; j++)
			{
				if(req->release[i][j] != 0)
				{
					req->curAlloc[i][j] -= req->release[i][j];
					resMatrix[j] += req->release[i][j];
					req->release[i][j] = 0;
				}
			}
		}
		break;
	}
}

//TODO: add in request checking and function to release enqueued processes

//deadlock detection test
void deadLock(Descriptor* req)
{
	if(written < 100000)
	{
		fprintf(stderr, "OSS running deadlock detection at time %d:%d\n",
			clock->seconds, clock->nano);
		written++;
	}	

	int i, j, k, m;

	//fill available resources table
	for(i = 0; i < MAX_RESOURCES; i++)
	{
		availMatrix[i] = resMatrix[i];
	}
		
	//calculate available resources
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		for(j = 0; j < MAX_RESOURCES; j++)
		{
			availMatrix[j] -= req->curAlloc[i][j];
		}
	}
	
	//mark empty current allocations
	int mark[MAX_PROCESSES];
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		int count = 0;
		for(j = 0; j < MAX_RESOURCES; j++)
		{
			if(req->curAlloc[i][j] == 0)
			{
				count++;
			}
			else
			{
				break;
			}
			
		}
		
		if(count == MAX_RESOURCES)
		{
			mark[i] = 1;
		}
	}

	//fill work array
	int work[MAX_RESOURCES];
	for(k = 0; k < MAX_RESOURCES; k++)
	{
		work[k] = availMatrix[k];
	}

	//check process that are less than or equal to work
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		int canProcess = 0;
		if(mark[i] != 1)
		{
			for(j = 0; j < MAX_RESOURCES; j++)
			{
				if(reqMatrix[i][j] <= work[j])
				{
					canProcess = 1;
				}
				else
				{
					canProcess = 0;
					break;
				}
			}
		
			if(canProcess)
			{
				mark[i] = 1;
			
				for(k = 0; k < MAX_RESOURCES; k++)
				{
					work[k] += req->curAlloc[i][k];
				}
			}
		}
	}

	//int deadlocked = 0;
	//display deadlocked processes
	if(written < 100000)
	{
		fprintf(stderr, "Processes deadlocked ");
		written++;
	}
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		int deadlocked = 0;
		if(mark[i] != 1)
		{
			deadlocked = 1;
		}

		if(deadlocked)
		{
			if(written < 100000)
			{
				fprintf(stderr, "P%d, ", i);
				written++;
			}
		}
		else
		{
			if(written < 100000)
			{
				fprintf(stderr,"No deadlock detected at P%d\n", i);
				written++;
			}
		}
	}

	if(written < 100000)
	{
		fprintf(stderr, "\nAttempting to resolve deadlock\n");
		written++;
	}
	
	//remove deadlocked processes
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		int dead = 0;
		if(mark[i] != 1)
		{
			dead = 1;
		}
		
		if(dead)
		{
			if(written < 100000)
			{
				fprintf(stderr, "Killing process P%d\n",i);
				written++;
				fprintf(stderr,"   Resources released are as follows:");
				written++;
				unaturalDeath++;
				launched--;
				death++;
				
			}
			for(j = 0; j < MAX_RESOURCES; j++)
			{
				if(written < 100000)
				{
					fprintf(stderr, " R%d:%d,", j, req->curAlloc[i][j]);
					written++;
				}
				resMatrix[j] += req->curAlloc[i][j];
				req->curAlloc[i][j] = 0;
				req->maxRes[i][j] = 0;
				req->release[i][j] = 0;
				req->request[i][j] = 0;
			}
		}
		req->shared[i] = 0;
		kill(req->pid[i], SIGTERM);
		req->pid[i] = 0;
		updatePid(i);
	}
	
	printMatrix(allocMatrix);
}

//test print to make sure that descriptors
//are being allocated correctly
void printDescriptor(Descriptor* req)
{
	int i, j, k, e, g;
	
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		printf("P%d Pid %d", i, req->pid[i]);
		printf("\n");
		for(j = 0; j < MAX_RESOURCES; j++)	
		{	
			printf("%d ", req->curAlloc[i][j]);	
		}
		printf("\n");
		for(k = 0; k < MAX_RESOURCES; k++)
		{
			printf("%d ", req->release[i][k]);
		}
		printf("\n");
		for(e = 0; e < MAX_RESOURCES; e++)
		{
			printf("%d ", req->request[i][e]);
		
		}
		printf("\n");
		for(g = 0; g < MAX_RESOURCES; g++)
		{
			printf("%d ", req->maxRes[i][g]);
		}
		
		printf("%d\n", req->shared[i]);
		printf("\n");
	}
}
//currently test but will be used for verbose printout	
void printMatrix(int arr[MAX_PROCESSES][MAX_RESOURCES])
{

	int i,j;

	for(i = 0; i < MAX_RESOURCES; i++)
	{
		if(written < 100000)
		{
			fprintf(stderr, "\tR%d", i);
			written++;
		}
	}
	if(written < 100000)
	{
		fprintf(stderr, "\n");
		written++;
	}
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		if(written < 100000)
		{
			fprintf(stderr, "P%d    ", i);
			written++;
		}
		for(j = 0; j < MAX_RESOURCES; j++)
		{
			if(written < 100000)
			{
				fprintf(stderr," %d\t", arr[i][j]);
				written++;
			}
		}
		if(written < 100000)
		{
			fprintf(stderr, "\n");
			written++;
		}
	}
}

//initialize everything and also test printouts for now
void allAtOnce()
{
	initMatrix(reqMatrix);
	initMatrix(allocMatrix);
	initVec(availMatrix);
	initVec(resMatrix);
	initDescriptor(req);
	fillTables(req, allocMatrix, resMatrix);
	//printMatrix(allocMatrix);
	//printMatrix(reqMatrix);

}





























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


int availMatrix[MAX_RESOURCES];
int resMatrix[MAX_RESOURCES];
int reqMatrix[MAX_PROCESSES][MAX_RESOURCES];
int allocMatrix[MAX_PROCESSES][MAX_RESOURCES];

int written;
int verbose;
FILE *fp;
struct Queue *queue;


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
		if(req->pid[i] == -1)
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
	srand(time(0));

	int i, j;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		req->pid[i] = -1;
		req->rpid[i] = 0;
		for(j = 0; j < MAX_RESOURCES; j++)
		{
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
			avail[i] = (rand() % 20) + 1;//available matrix
			alloc[i][j] = req->curAlloc[i][j];//allocation matrix
		}
	}
}

//update request matrix with latest requests
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
	if(written < 100000 && verbose == 1)
	{
		fprintf(fp, "OSS Terminating P%d at time %d:%d\n",i, clock->seconds, clock->nano);
		written++;
	}

	//naturalDeath++;

	int k;
			
	for(k = 0; k < MAX_RESOURCES; k++)
	{
		availMatrix[k] += req->curAlloc[i][k];
	}
	updatePid(i);
}

//after terminating a pid, update allocations
//and request table
void updatePid(int pos)
{
	srand(time(0));
	
	int i, j;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		if(req->pid[i] == pos)
		{
			req->pid[i] = -1;
			req->rpid[i] = 0;
			for(j = 0; j < MAX_RESOURCES; j++)
			{
				req->curAlloc[i][j] = (rand()% 10) + 1;
				allocMatrix[i][j] = req->curAlloc[i][j];
				req->release[i][j] = 0;
				req->request[i][j] = 0;
			}
		}
		req->shared[i] = (((rand() % 100) + 1) < 20) ? 1 : 0;
	}
}

void releaseResource(Descriptor *req, int pid)
{
	if(written < 100000 && verbose == 1)
	{
		fprintf(fp, "OSS has acknowledged process P%d requesting to release resources at time %d:%d\n",
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
					allocMatrix[i][j] = req->curAlloc[i][j];
					availMatrix[j] += req->release[i][j];
					req->release[i][j] = 0;
				}
			}
		}
		break;
	}
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
		
		printf("%d\n", req->shared[i]);
		printf("\n");
	}
}
//currently test but will be used for verbose printout	
void printMatrix(int arr[MAX_PROCESSES][MAX_RESOURCES])
{

	int i,j;
	if(written < 100000 && verbose == 1)
	{
		fprintf(fp, "\n line count = %d", written);
		written++;
	}

	for(i = 0; i < MAX_RESOURCES; i++)
	{
		if(written < 100000 && verbose == 1)
		{
			fprintf(fp, "\tR%d", i);
			written++;
		}
	}
	if(written < 100000 && verbose == 1)
	{
		fprintf(fp, "\n");
		written++;
	}
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		if(written < 100000 && verbose == 1)
		{
			fprintf(fp, "P%d    ", i);
			written++;
		}
		for(j = 0; j < MAX_RESOURCES; j++)
		{
			if(written < 100000 && verbose == 1)
			{
				fprintf(fp," %d\t", arr[i][j]);
				written++;
			}
		}
		if(written < 100000 && verbose == 1)
		{
			fprintf(fp, "\n");
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
	fillTables(req, allocMatrix, availMatrix);
}





























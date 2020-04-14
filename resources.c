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

int deadlockTerm = 0;
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
	Descriptor *array = req;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		if(array->pid[i] == pid)
		{
			return i;
		}
		array++;
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
	Descriptor* array;
	array = req;
	int i, j;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		array->pid[i] = 0;

		for(j = 0; j < MAX_RESOURCES; j++)
		{
			array->maxRes[j] = 0;
			array->curAlloc[j] = (rand() % 10) + 1;
			array->release[j] = 0;
			array->request[j] = 0;
		}

		array->time.nano = 0;
		array->time.seconds = 0;
		//zero is not shared, 1 is shared
		array->shared = (((rand() % 100) + 1) < 20) ? 1 : 0;
		array++;
	}
}

//fill request and allocation static tables
//also fill max requests for descriptors
//allocation + request table values.
void fillTables(Descriptor* req, int alloc[MAX_PROCESSES][MAX_RESOURCES],int avail[MAX_RESOURCES])
{
	int i, j;
	for(i = 0; i < MAX_RESOURCES; i++)
	{
		avail[i] = (rand() % 10) + 1;
	}
	Descriptor* iter;
	iter = req;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		for(j = 0; j < MAX_RESOURCES; j++)
		{
			alloc[i][j] = iter->curAlloc[j];
			iter->maxRes[j] = avail[j];
		}
		iter++;
	}
}

void updateRequest(Descriptor *req, int array[MAX_PROCESSES][MAX_RESOURCES])
{
	int i, j;
	Descriptor *iter = req;
	
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		for(j = 0; j < MAX_RESOURCES; j++)
		{
			array[i][j] += iter->request[j];
		}
		iter++;
	}
}

//terminate pid and release allocations back
//to available.
void offThePid(Descriptor *req, int pid)
{
	int i = getPos(req, pid);
	
	fprintf(stderr, "OSS Terminating P%d at time %d:%d\n",i, clock->seconds, clock->nano);

	Descriptor *array = req;
	int j, k;
	for(j = 0; j < MAX_PROCESSES; j++)
	{
		if(j == i)
		{
			array->time.nano = 0;
			array->time.seconds = 0;
			array->shared = 0;
			for(k = 0; k < MAX_RESOURCES; k++)
			{
				array->maxRes[k] = 0;
				availMatrix[k] += array->curAlloc[k];
				array->curAlloc[k] = 0;
				array->release[k] = 0;
				array->request[k] = 0;
				reqMatrix[j][k] = 0;
			}
			kill(array->pid[j], SIGTERM);
			array->pid[j] = 0;
		}
		array++;
	}
	//updatePids();
}


/*void updatePids()
{
	int i, j;
	for(i = 0; i < MAX_PROCESSES; i++)
	{				
	
*/
//deadlock detection test
void deadLock(Descriptor* req)
{
	
 	Descriptor *array;
	array = req;

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
			availMatrix[j] -= array->curAlloc[j];
		}
		array++;
	}

	Descriptor *check = req;
	int mark[MAX_PROCESSES];
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		int count = 0;
		for(j = 0; j < MAX_RESOURCES; j++)
		{
			if(check->curAlloc[j] == 0)
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
		check++;
	}

	int work[MAX_RESOURCES];
	for(k = 0; k < MAX_RESOURCES; k++)
	{
		work[k] = availMatrix[k];
	}

	//check process that are less than or equal to work
	Descriptor *ptr = req;
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
				mark[i] == 1;
			
				for(k = 0; k < MAX_RESOURCES; k++)
				{
					work[k] += req->curAlloc[k];
				}
			}
		}
	}

	int deadlocked = 0;
	
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		if(mark[i] != 1)
		{
			deadlocked = 1;
		}

		if(deadlocked)
		{
			printf("Deadlock detected with P%d\n", i);
		}
		else
		{
			printf("No deadlock detected at P%d\n", i);
		}
	}
	
	
}

//test print to make sure that descriptors
//are being allocated correctly
void printDescriptor(Descriptor* req)
{
	Descriptor* array;
	array = req;
	int i, j, k, e, g;
	
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		printf("P%d Pid %d", i, array->pid[i]);
		printf("\n");
		for(j = 0; j < MAX_RESOURCES; j++)	
		{	
			printf("%d ", array->curAlloc[j]);	
		}
		printf("\n");
		for(k = 0; k < MAX_RESOURCES; k++)
		{
			printf("%d ", array->release[k]);
		}
		printf("\n");
		for(e = 0; e < MAX_RESOURCES; e++)
		{
			printf("%d ", array->request[e]);
		
		}
		printf("\n");
		for(g = 0; g < MAX_RESOURCES; g++)
		{
			printf("%d ", array->maxRes[g]);
		}
		
		printf("\n%d\n", array->time.nano);
		printf("%d\n", array->time.seconds);
		printf("%d\n", array->shared);
		printf("\n");
		array++;
	}
}
//currently test but will be used for verbose printout	
void printMatrix(int arr[MAX_PROCESSES][MAX_RESOURCES])
{

	int i,j;

	for(i = 0; i < MAX_RESOURCES; i++)
	{
		printf("\tR%d", i);
	}
	printf("\n");
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		printf("P%d    ", i);
		for(j = 0; j < MAX_RESOURCES; j++)
		{
			printf(" %d\t", arr[i][j]);
		}
		printf("\n");
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
	printMatrix(allocMatrix);
	printMatrix(reqMatrix);

}





























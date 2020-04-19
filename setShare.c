//===============================================================
//Date:		April 9,2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 5
//File:		setShare.c
//==============================================================
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

//shared memory keys, id's and sizes
const key_t clockKey = CLOCK;
const key_t reqKey = REQ;
const key_t semKey = SEM;

int clockID = 0;
int reqID = 0;
int semID = 0;

const size_t clockSize = sizeof(Clock);
const size_t reqSize = sizeof(Descriptor);
const size_t semSize = sizeof(sem_t);

Clock *clock = NULL;
Descriptor* req = NULL;
sem_t* sem = NULL;
//Message queue keys and id's
key_t msgTo;
key_t msgFrm;

int toOSS;
int toUSR;

FILE *fp;
int naturalDeath;
int unaturalDeath;	
sem_t* getSem(const key_t key, const size_t size, int* shmid)
{
	*shmid = shmget(key, size, PERM);
	if(*shmid < 0)
	{
		perror("ERROR: shmget failed (sem_t)\n");
		cleanAll();
		exit(1);
	}

	sem_t* temp = shmat(*shmid, NULL, 0);
	if(temp == (sem_t *)-1)
	{
		perror("ERROR shmat failed (sem_t)\n");
		cleanAll();
		exit(1);
	}

	if(sem_init(temp, 1, 1) == -1)
	{
		perror("ERROR: sem_init failed\n");
		cleanAll();
		exit(1);
	}
	
	return temp;
}

void *getMem(const key_t key, const size_t size, int* shmid)
{
	*shmid = shmget(key, size, PERM);
	if(*shmid < 0)
	{
		perror("ERROR: shmget failed (getMem)\n");
		cleanAll();
		exit(1);
	}

	void* temp = shmat(*shmid, NULL, 0);
	if(temp == (void *)-1)
	{
		perror("ERROR: shmat failed (getMem)\n");
		cleanAll();
		exit(1);
	}

	return temp;
}

void initMem()
{
	clock = (Clock*)getMem(clockKey, clockSize, &clockID);
	req = (Descriptor*)getMem(reqKey, reqSize, &reqID);
	sem = getSem(semKey, semSize, &semID);
}

void message()
{
	msgTo = MSG1;

	toUSR = msgget(msgTo, PERM);
	
	if(toUSR == -1)
	{
		perror("ERROR: msgget(toUSR)\n");
		clearMessage();
		exit(1);
	}

	msgFrm = MSG2;

	toOSS = msgget(msgFrm, PERM);

	if(toOSS == -1)
	{
		perror("ERROR: mssget(msg2)\n");
		clearMessage();
		exit(1);
	}
}

void clearMessage()
{
	msgctl(toUSR, IPC_RMID, NULL);
	msgctl(toOSS, IPC_RMID, NULL);
}

void detachMem()
{
	shmdt(clock);
	shmdt(req);
	shmdt(sem);
}

void removeMem(int shmid)
{
	int temp = shmctl(shmid, IPC_RMID, NULL);
	if(temp == -1)
	{
		perror("OSS: shmct failed\n");
		exit(1);
	}
}

void cleanAll()
{
	if(clockID > 0)
		removeMem(clockID);
	if(reqID > 0)
		removeMem(reqID);
	if(semID > 0)
		removeMem(semID);
}

void initClock(Clock* mainClock)
{
	mainClock->nano = 0;
	mainClock->seconds = 0;
}


void timesUp(int sig)
{
	char msg[] = "\nProgram has reached 3 seconds\n";
	int msgSize = sizeof(msg);
	write(STDERR_FILENO, msg, msgSize);

	int i;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		if(req->pid[i] != -1)
		{
			if(kill(req->pid[i], SIGTERM) == -1)
			{
				perror("ERROR: OSS: SIGTERM(timesUp)\n");
			}
		}
	}
	fclose(fp);
	printf("Terminations = %d\n", naturalDeath);
	printf("Deadlock terminations = %d\n", unaturalDeath);
	clearMessage();
	cleanAll();
	exit(0);
}

void killAll(int sig)
{
	char msg[] = "\nCaught CTRL+C\n";
	int msgSize = sizeof(msg);
	write(STDERR_FILENO, msg, msgSize);

	int i;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		if(req->pid[i] != -1)
		{
			if(kill(req->pid[i], SIGTERM) == -1)
			{
				perror("ERROR: OSS: SIGTERM(killAll)\n");
			}
		}
	}
	clearMessage();
	cleanAll();
	exit(0);
}

void endProcesses()
{
	int i;
	for(i = 0; i < MAX_PROCESSES; i++)
	{
		if(req->pid[i] != -1)
		{
			if(kill(req->pid[i], SIGTERM) == -1)
			{
				perror("ERROR: OSS: SIGTERM(endProcesses)\n");
			}
		}
	}
	//clearMessage();
	//cleanAll();
}

void destroy(int sig)
{
	shmdt(clock);
	shmdt(req);
	shmdt(sem);
	exit(12);
}


void insert(struct Queue **start, int pid)
{
	if(*start == NULL)
	{
		struct Queue *new_queue = (struct Queue *)malloc(sizeof(struct Queue));
		new_queue->pid = pid;
		new_queue->next = new_queue->prev = new_queue;
		*start = new_queue;
		return;
	}

	struct Queue *last = (*start)->prev;

	struct Queue *new_queue = (struct Queue*)malloc(sizeof(struct Queue));
	
	new_queue->pid = pid;

	new_queue->next = *start;

	(*start)->prev = new_queue;

	new_queue->prev = last;

	last->next = new_queue;
}


int search(struct Queue *start, int pid)
{
	struct Queue *temp = start;

	int flag = 0;

	if(temp == NULL)
	{
		return -1;
	}
	else
	{
		while(temp->next != start)
		{
			if(temp->pid == pid)
			{
				flag = 1;
				break;
			}
			temp = temp->next;
		}

		if(temp->pid == pid)
		{
			flag = 1;
		}
		
		if(flag == 1)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
}

void deleteQ(struct Queue **start, int pid)
{
	if(*start == NULL)
	{
		return;
	}

	struct Queue *curr = *start, *prev_1 = NULL;

	while(curr->pid != pid)
	{
		if(curr->next == *start)
		{
			return;
		}
		prev_1 = curr;
		curr = curr->next;
	}

	if(curr->next == *start && prev_1 == NULL)
	{
		(*start) = NULL;
		free(curr);
		return;
	}

	if(curr == *start)
	{
		prev_1 = (*start)->prev;
		
		*start = (*start)->next;
		
		prev_1->next = *start;

		(*start)->prev = prev_1;

		free(curr);
	}
	else if(curr->next == *start)
	{
		struct Queue *temp = curr->next;

		prev_1->next = temp;

		temp->prev = prev_1;

		free(curr);
	}
}

void display(struct Queue *start)
{
	if(start == NULL)
	{
		fprintf(stderr, "\nQueue list is empty\n");
		return;
	}

	struct Queue *temp = start;

	while(temp->next != start)
	{
		fprintf(stderr, "%d ", temp->pid);
		temp = temp->next;
	}
	fprintf(stderr, "%d ", temp->pid);
}

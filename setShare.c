//===============================================================
//Date:		April 9,2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 5
//File:		setShare.c
//==============================================================

#include "share.h"

const key_t clockKey = CLOCK;
const key_t msgKey = MSG;
const key_t reqKey = REQ;
const key_t semKey = SEM;

int clockID = 0;
int msgID = 0;
int reqID = 0;
int semID = 0;

const size_t clockSize = sizeof(Clock);
const size_t msgSize = sizeof(Msg);
const size_t reqSize = sizeof(ReqCheck);
const size_t semSize = sizeof(sem_t);

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

void detachMem()
{
	if(clockID > 0)
		shmdt(&clockID);
	if(msgID > 0)
		shmdt(&msgID);
	if(reqID > 0)
		shmdt(&reqID);
	if(semID > 0)
		shmdt(&semID);
}

void removeMem(int* shmid)
{
	int temp = shmctl(*shmid, IPC_RMID, NULL);
	if(temp == -1)
	{
		perror("OSS: shmct failed\n");
		exit(1);
	}
}

void cleanAll()
{
	if(clockID > 0)
		removeMem(&clockID);
	if(msgID > 0)
		removeMem(&msgID);
	if(reqID > 0)
		removeMem(&reqID);
	if(semID > 0)
		removeMem(&semID);
}

void initClock(Clock* mainClock)
{
	mainClock->nano = 0;
	mainClock->seconds = 0;
}

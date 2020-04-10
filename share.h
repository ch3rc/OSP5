//===================================================================================
//Date:		April 9,2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 5
//File:		share.h
//===================================================================================

#ifndef SHARE_H
#define SHARE_H

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
#include <semaphore.h>

#define MAX_PROCESSES 18

#define PERM (IPC_CREAT | 0666)

#define CLOCK 0x11111111
#define MSG 0x22222222
#define REQ 0x33333333
#define SEM 0x44444444

//keys
extern const key_t clockKey;
extern const key_t msgKey;
extern const key_t reqKey;
extern const key_t semKey;

//id's
extern int clockID;
extern int msgID;
extern int reqID;
extern int semID;

//sizes
extern const size_t clockSize;
extern const size_t msgSize;
extern const size_t reqSize;
extern const size_t semSize;


typedef struct{
	
	unsigned int nano;
	unsigned int seconds;

}Clock;

typedef struct{
	
	int state;
	pid_t usrPid;
	int index;

}Msg;

typedef struct{

	int state;
	pid_t reqPid;
	int nano;
	int sec;

}Req;

typedef struct{

	Req kidReqs[MAX_PROCESSES];

}ReqCheck;

sem_t* getSem(const key_t, const size_t, int* );
void* getMem(const key_t, const size_t, int* );
void detachMem();
void removeMem(int* );
void cleanAll();
void initClock(Clock* );

#endif

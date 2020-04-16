//===================================================================================
//Date:		April 9,2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 5
//File:		share.h
//===================================================================================

#ifndef SHARE_H
#define SHARE_H

#include "queue.h"

#define MAX_PROCESSES 18
#define MAX_RESOURCES 20

#define PERM (IPC_CREAT | 0666)

#define CLOCK 0x11111111
#define REQ 0x22222222
#define SEM 0x33333333
#define MSG1 0x44444444
#define MSG2 0x55555555

//total available resources
extern int availMatrix[MAX_RESOURCES];
//total resources
extern int resMatrix[MAX_RESOURCES];
//store requests from processes
extern int reqMatix[MAX_PROCESSES][MAX_RESOURCES];
//store the current resource allocation of processes
extern int allocMatrix[MAX_PROCESSES][MAX_RESOURCES];

extern int unaturalDeath;
extern int naturalDeath;
extern int launched;
extern int death;

//keys
extern const key_t clockKey;
extern const key_t reqKey;
extern const key_t semKey;

//id's
extern int clockID;
extern int toOSS;
extern int toUSR;
extern int reqID;
extern int semID;

//sizes
extern const size_t clockSize;
extern const size_t reqSize;
extern const size_t semSize;

typedef struct{
	
	unsigned int nano;
	unsigned int seconds;

}Clock;


typedef struct{
	
	pid_t pid[MAX_PROCESSES];
	int maxRes[MAX_PROCESSES][MAX_RESOURCES];
	int curAlloc[MAX_PROCESSES][MAX_RESOURCES];
	int release[MAX_PROCESSES][MAX_RESOURCES];
	int request[MAX_PROCESSES][MAX_RESOURCES];
	int shared[MAX_PROCESSES];

}Descriptor;


extern Clock* clock;
extern Descriptor* req;
extern sem_t* sem;	

//in setShare.c
sem_t* getSem(const key_t, const size_t, int* );
void* getMem(const key_t, const size_t, int* );
void message();
void initMem();
void clearMessage();
void detachMem();
void removeMem(int* );
void cleanAll();
void initClock(Clock* );
void killAll(int);
void timesUp(int);
void destroy(int);
void endProcesses();

//in resources.c
void tickClock(Clock*, unsigned int);
int compare(Clock *, Clock *);
void initMatrix(int arr[MAX_PROCESSES][MAX_RESOURCES]);
void initVector(int arr[MAX_RESOURCES]);
void printMatrix(int arr[MAX_PROCESSES][MAX_RESOURCES]);
void initDescriptor(Descriptor*);
void printDescriptor(Descriptor*);
int getPos(Descriptor*,int);
int findSpot();
void fillTables(Descriptor*, int arr[MAX_PROCESSES][MAX_RESOURCES], int arr2[MAX_RESOURCES]);
void banker(Descriptor*);
void deadLock(Descriptor*, struct Queue *);
void offThePid(Descriptor*, int);
void updatePid(int);
void checkRequest(struct Queue *, Descriptor *, int);
void checkAgain(struct Queue *, Descriptor *, int);
#endif

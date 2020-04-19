//=============================================================
//Date:		April 10,2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 5
//File:		queue.h
//=============================================================



#ifndef QUEUE_H
#define QUEUE_H

//Represent a queue
struct Queue{

	int pid;
	struct Queue *next;
	struct Queue *prev;
};

//insert pid into queue for sleep state
void insert(struct Queue **, int);
//search for pid if in sleep state to see
//if request can be met
int search(struct Queue *, int);
//remove pid from queue if request can
//be met
void deleteQ(struct Queue **, int);
//printout
void display(struct Queue *);



#endif

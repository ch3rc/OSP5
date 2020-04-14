//=============================================================
//Date:		April 10,2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 5
//File:		queue.h
//=============================================================
/*code used from https://geeksforgeeks.org/queue-set-1introduction
 * -and-array-implementation/ */


#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

//Represent a queue
struct Queue
{
	int front, rear, size;
	unsigned capacity;
	int *array;
};

//Create a queue of given capacity, starts at 0
struct Queue* createQueue(unsigned capacity)
{
	struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
	queue->capacity = capacity;
	queue->front = queue->size = 0;
	queue->rear = capacity - 1;
	queue->array = (int*)malloc(queue->capacity * sizeof(int));
	return queue;
}

//Queue is full when size becomes equal to the capacity
int isFull(struct Queue* queue)
{ return (queue->size == queue->capacity); }

//Queue is empty when size is 0
int isEmpty(struct Queue * queue)
{return (queue->size == 0); }

//add an item to the queue, it changes rear and size
void enqueue(struct Queue* queue, int item)
{
	if(isFull(queue))
		return;
	queue->rear = (queue->rear + 1) % queue->capacity;
	queue->array[queue->rear] = item;
	queue->size = queue->size + 1;
}

//remove an item from queue, and change size of front
//and rear
int dequeue(struct Queue* queue)
{
	if(isEmpty(queue))
		return INT_MIN;
	int item = queue->array[queue->front];
	queue->front = (queue->front + 1) % queue->capacity;
	queue->size = queue->size - 1;
	return item;
}


#endif

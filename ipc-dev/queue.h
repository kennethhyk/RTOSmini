#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX 6

typedef struct {
	int intArray[MAX];
	int front;
	int rear;
	int itemCount;
} Queue;

void q_init(Queue *q){
	// memset(q->intArray, 0, sizeof(int)*MAX);
	for(int i = 0; i < 6; i++){
    	q->intArray[i] = 0;
    }
	q->front = 0;
	q->rear = -1;
	q->itemCount = 0;
}

int q_peek(Queue *q) {
	return q->intArray[q->front];
}

bool q_isEmpty(Queue *q) {
	return q->itemCount == 0;
}

bool q_isFull(Queue *q) {
	return q->itemCount == MAX;
}

int q_size(Queue *q) {
	return q->itemCount;
}  

void q_insert(int data, Queue *q) {
	printf("q inserting %d\n", data);
	if(!q_isFull(q)) {
		if(q->rear == MAX-1) {
			q->rear = -1;
		}
		q->intArray[++(q->rear)] = data;
		printf("array at %d: %d\n", q->rear,q->intArray[q->rear]);
		q->itemCount++;
	}
}

int q_removeData(Queue *q) {
	int data = q->intArray[(q->front)++];

	if(q->front == MAX) {
		q->front = 0;
	}

	q->itemCount--;
	return data;  
}

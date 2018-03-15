#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX 6

typedef struct {
	PID pid;
	int msg;
} Msg_Des;

typedef struct {
	int msgArray[MAX];
	// PID pidArray[MAX];
	int front;
	int rear;
	int itemCount;
} Queue;

void q_init(Queue *q){
	printf("initializing");
	memset(q->msgArray, 0, sizeof(int)*MAX);
	// memset(q->pidArray, 0, sizeof(PID)*MAX);
	q->front = 0;
	q->rear = -1;
	q->itemCount = 0;
}

int q_peek(Queue *q) {
	return q->msgArray[q->front];
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

void q_insert(PID pid, int msg, Queue *q) {
	if(!q_isFull(q)) {
		if(q->rear == MAX-1) {
			q->rear = -1;
		}
		// q->pidArray[++(q->rear)] = pid;
		q->msgArray[++(q->rear)] = msg;
		// printf("inserting %d %d\n", q->pidArray[(q->rear)], q->msgArray[(q->rear)]);
		q->itemCount++;
	}
}

int q_removeData(Queue *q) {
	int data = q->msgArray[(q->front)++];

	if(q->front == MAX) {
		q->front = 0;
	}

	q->itemCount--;
	return data;  
}

#include <string.h>
#include <stdio.h>
#include "os.h"
#include "queue.h"

typedef struct IPC_MAILBOX 
{
	int h;
} MB;

void hi() {
    puts("Hello world! hahahhh");
    Queue *q;
    q = (Queue *)malloc(sizeof(Queue));
    q_init(q);
    q_insert(1, q);
    q_insert(2, q);
    q_insert(3, q);
    q_removeData(q);
    q_insert(4, q);
    q_insert(1, q);

    while(!q_isEmpty(q)) {
    	int n = q_removeData(q);
    	printf("%d ", n);
    }   
    printf("\n");
}

// int main(){
// 	PORTB = 0b11111111;
// 	return 0;
// }
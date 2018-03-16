#include <string.h>
#include <stdio.h>
#include "queue.h"
#include "os.h"

typedef struct IPC_MAILBOX 
{
	int status;
    Queue message_q;
} IPC_MAILBOX;

// void Msg_Send( PID  id, MTYPE t, unsigned int *v ) {
//     //find corresponding PD
//     Process[0];
//     printf("SENDING TO %d", id);
//     printf("CHECKING RECVIEVER STATUS");
//     while(1){
//         //send block
//     }

//     while(1){
//         //recv block
//     }
// }

// PID  Msg_Recv( MASK m,           unsigned int *v ) {

// }

// void Msg_Rply( PID  id,          unsigned int r ) {

// }

// void Msg_ASend( PID  id, MTYPE t, unsigned int v ) {

// }

void hi() {
    printf("hello world");
    Queue *q;
    q = (Queue *)malloc(sizeof(Queue));
    q_init(q);
    q_insert(0,1, q);
    q_insert(0,2, q);
    q_insert(0,3, q);
    q_removeData(q);
    q_insert(0,4, q);
    q_insert(0,1, q);

    while(!q_isEmpty(q)) {
     int n = q_removeData(q);
     printf("%d ", n);
    }   
    printf("\n");
}
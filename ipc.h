#include <string.h>
#include <stdio.h>
// #include "queue.c"
// #include "os.h"
// #include "kernel.h"

// typedef struct IPC_MAILBOX 
// {
// 	int status;
//     ipc_queue msg_q;
// } IPC_MAILBOX;

// void Msg_Send( PID  id, MTYPE t, unsigned int *v ) {
//     //test init
//     Process[id].pid = id;
//     //find corresponding PD
//     // int recv_process;
//     // printf("SENDING TO %d\n", id);
//     // printf("CHECKING RECIEVER STATUS\n");
//     // for(int i = 0 ;i < 4; i++) {
//     //   if(id == Process[i].pid) {
//     //       recv_process = i;
//     //       break;
//     //   }
//     // }
//     // printf("ENTERING SEND BLOCK\n");
//     // while(Process[recv_process].mailbox.status != 1){
//     //   //send block
//     //   Task_Next();
//     // }
//     // printf("LEAVING SEND BLOCK\n");
//     // // q_insert(Cp->pid, 9, &Process[recv_process].mailbox.message_q);
    
//     // printf("just inserted %d\n" ,q_removeData(&Process[recv_process].mailbox.message_q));
//     // // for(int j = 0;j < 6;j++){
//     // //   int mg = q_peek(&Process[j].mailbox.message_q);
//     // //   printf("%d msg: %d\n", j, mg);
//     // // }
//     // printf("ENTERING RECV BLOCK\n");
//     // Task_Next();
//     // while(1){
//     //     //recv block
//     // }
// }

// PID  Msg_Recv( MASK m,           unsigned int *v ) {
//   // Cp->mailbox.status = 1;
//   // while(q_size(&(Cp->mailbox.message_q)) == 0) {
//   //   // recv block
//   //   printf("recv block\n");
//   //   Task_Next();
//   // }
//   // printf("leaving recv block\n");
//   // int m2 = q_removeData(&Cp->mailbox.message_q);
//   // printf("i recieved %d\n", m2);

// }

// void Msg_Rply( PID  id,          unsigned int r ) {

// }

// void Msg_ASend( PID  id, MTYPE t, unsigned int v ) {

// }
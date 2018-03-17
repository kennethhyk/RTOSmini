void sender()
{
  unsigned int v = 9;
  printf("sender sent: %d\n", v);
  Msg_Send(3, GET, &v);
  printf("sender recieved: %d\n", v);

  // unsigned int v = 9;
  // Msg_ASend( 2, PUT, v );
  // printf("sender asend: %d\n", v);
}

void reciever()
{
  unsigned int v = 0;
  PID reply_pid = Msg_Recv(ALL, &v);
  printf("reciever recieved: %d\n", v);
  Msg_Rply(reply_pid, 4);

  // unsigned int v = 0;
  // PID reply_pid = Msg_Recv( PUT, &v );
  // printf("reciever recieved: %d\n", v);
  // Msg_Rply( reply_pid, 4);
}

void a_main(void) {
  Task_Create_RR(sender, 0);
  Task_Create_RR(reciever, 0);
}

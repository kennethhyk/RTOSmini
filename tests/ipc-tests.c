void sender()
{
  printf("sender\n");
  unsigned int v = 9;
  Msg_Send(3, GET, v);
  printf("sender sent: %d\n", v);
  Task_Next();
  // unsigned int v = 9;
  // Msg_ASend( 2, PUT, v );
  // printf("sender asend: %d\n", v);
}

void receiver()
{
  printf("recver\n");
  unsigned int v = 0;
  // printf("reciever entered\n");
  PID reply_pid = Msg_Recv(ALL, &v);
  printf("reciever recieved: %d\n", v);
  Msg_Rply(reply_pid, 4);

  // unsigned int v = 0;
  // PID reply_pid = Msg_Recv( PUT, &v );
  // printf("reciever recieved: %d\n", v);
  // Msg_Rply( reply_pid, 4);
}

void a_main() {
  printf("helloworld\n");
  Task_Create_System(sender, 0);
  Task_Create_System(receiver, 0);
}
void sender()
{
  unsigned int send_msg = 9;
  unsigned int v = send_msg;
  PID pid = 5;
  printf("sender sending to task(pid:%u): ----> %u\n", pid, v);
  Msg_Send(pid, GET, &v);
  printf("sender received reply: <---- %u\n", v);
}

void receiver()
{
  unsigned int v = 0;
  unsigned int reply_msg = 4;
  PID reply_pid = Msg_Recv(GET, &v);
  printf("receiver recieved from sender(pid:%u): <---- %u\n", reply_pid, v);
  printf("starting replying to sender(pid:%u): ----> %u\n", reply_pid, reply_msg);
  Msg_Rply(reply_pid, reply_msg);
}

void a_main() {
  printf("Receiver does not exist\nSHOULD ONLY SEE THE SENDER SENDS\n\n");
  Task_Create_System(sender, 0);
  Task_Create_System(receiver, 0);
}
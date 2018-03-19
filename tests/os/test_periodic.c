void Ping()
{
  printf("Started Ping\n");
  printf("Finished Ping\n");
}

void Pong()
{
  printf("started pong\n");
  printf("finished pong\n");
}

void Ding()
{
  printf("Started Ding\n");
  printf("Finished Ding\n");
}

void a_main(){
    Task_Create_Period(Ping, 0, 100, 5, 5);
    Task_Create_Period(Pong, 0, 30, 5, 7);
    Task_Create_Period(Ding, 0, 15, 5, 3);
    // expect to see each task start and finish in order of:
    // ding, pong, ping
}
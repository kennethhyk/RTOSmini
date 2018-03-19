void Ping()
{
  printf("Executed Ping\n");
}

void Pong()
{
  printf("Executed pong\n");
}

void Ding()
{
  printf("Executed Ding\n");
}

void a_main(){
    Task_Create_Period(Pong, 0, 10, 5, 15);
    Task_Create_Period(Ding, 0, 10, 5, 20);
    // expect to see each task start and finish in order of:
    // ding, pong, ping
}
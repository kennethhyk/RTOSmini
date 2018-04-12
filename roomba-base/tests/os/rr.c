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

void Dong()
{
  printf("started Dong\n");
  printf("finished Dong\n");
}

void a_main(){
    Task_Create_RR(Ping, 0);
    Task_Create_RR(Pong, 0);
    Task_Create_RR(Ding, 0);
    Task_Create_RR(Dong, 0);
    // expect to see each task start and finish in order 
}
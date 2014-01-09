// threads_buffers.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define COUNT 30
#define SHMKEY 321

int process_sig;
int proc_s;
int proc_c;
int count;
int shared_mem;
int *shm_addr;

union semun {
  int              val;    /* Value for SETVAL */
  struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
  unsigned short  *array;  /* Array for GETALL, SETALL */
  struct seminfo  *__buf;  /* Buffer for IPC_INFO
                              (Linux specific) */
};


void p(int semid, int semno)
{
  struct sembuf sop;
  sop.sem_num = semno;
  sop.sem_op = -1;
  sop.sem_flg = 0;
  semop(semid, &sop, 1);
}

void v(int semid, int semno)
{
  struct sembuf sop;
  sop.sem_num = semno;
  sop.sem_op = 1;
  sop.sem_flg = 0;
  semop(semid, &sop, 1);
}

void server(void) 
{
  int i, res;
  shared_mem = shmget(SHMKEY, 10 * sizeof(int), IPC_CREAT | 0777);
  shm_addr = (int *)shmat(shared_mem, NULL, 0);

  count = COUNT;
  printf("Server started. %d\n", count);

  while (count > 0)
  {
    printf("Server loop: count %d\n", count);
    res = 1;
    for (i=0; i<count; i++) 
      res*=2;
    shm_addr[0] = res;
    shm_addr[1] = 0;
    printf("Server ready: %d\n", res);
    v(process_sig, 1);
    count--;
    sleep(1);
    p(process_sig, 0);
  }
  shm_addr[1] = 1;
  v(process_sig, 1);
  //shmctl(shared_mem, IPC_RMID, 0);
  printf("Server exiting.\n");
  exit(0);
}

void client(void)
{
  int i = 1;
  shared_mem = shmget(SHMKEY, 10 * sizeof(int), 0777);
  shm_addr = (int *)shmat(shared_mem, NULL, 0);
  printf("Client started. \n");

  while (1)
  {
    printf("Client loop: %d\n", i);
    p(process_sig, 1);
    if (shm_addr[1]) break;
    printf("Result received: %d\n", shm_addr[0]);
    v(process_sig, 0);
    i++;
  }
  shmctl(shared_mem, IPC_RMID, 0);
  printf("Client exiting.\n");
  exit(0);
}

void ls(void)
{
  sleep(COUNT + 1);
  printf("Listing files:\n");
  execl("/bin/ls", "-a");
}

int main(void)
{ 
  int i;
  union semun arg;

  process_sig = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
  arg.val = 0;
  semctl(process_sig, 0, SETVAL, arg);
  semctl(process_sig, 1, SETVAL, arg);

  printf("Starting server.\n");
  while ((i = fork()) == -1) printf("Cannot fork P1. Retrying.\n");
  if (i == 0) server();
  printf("Starting client.\n");
  while ((i = fork()) == -1) printf("Cannot fork P2. Retrying.\n");
  if (i == 0) client();
  i = fork();
  if (i == 0) ls();

  printf("Waiting for child processes...\n");
  wait(0);
  wait(0);
  wait(0);
  semctl(process_sig, 0, IPC_RMID, 0);
  semctl(process_sig, 1, IPC_RMID, 0);
  printf("Finished.\n");
  return (0);
}

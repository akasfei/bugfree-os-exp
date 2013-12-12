// tickets.c

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define STOCKCOUNT 10

int thread_sig,
    stock,
    sold;

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

void sell(void) 
{
  while (stock > 0)
  {
    stock--;
    printf("Stock: %d\n", stock);
    v(thread_sig, 0);
    p(thread_sig, 0);
  }
}

void buy(void)
{
  while (sold < STOCKCOUNT)
  {
    p(thread_sig, 0);
    sold++;
    printf("%d sold, %d left.\n", sold, stock);
    v(thread_sig, 0);
  }
}

int main(void)
{ 
  pthread_t tid1, tid2;
  int i, thread1, thread2;
  union semun arg;
  stock = STOCKCOUNT;
  sold = 0;

  thread_sig = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
  arg.val = 0;
  semctl(thread_sig, 0, SETVAL, arg);

  thread1 = pthread_create(&tid1, NULL, (void *) sell, NULL);
  thread2 = pthread_create(&tid2, NULL, (void *) buy, NULL);
  if(thread1 != 0 || thread2 != 0){
    printf ("Create pthread error!\n");
    return (1);
  }

  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  semctl(thread_sig, 0, IPC_RMID, 0);
  printf("Finished.\n");
  return (0);
}

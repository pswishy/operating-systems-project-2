#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>


void  ParentProcess(int *);
void  ChildProcess(int *);
void  depositMoney(int *);
sem_t *mutex;

int main(int argc, char **argv)
{
  int fd, i,zero=0;
  int bankAccount = 0;
  int ShmID;
  int *ShmPTR;

  //* create, initialize semaphore */
  if ((mutex = sem_open("example._semaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
    perror("semaphore initilization");
    exit(1);
  }

  ShmID = shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0666);
  if (ShmID < 0) {
    printf("*** shmget error (server) ***\n");
    exit(1);
  }

  ShmPTR = (int *) shmat(ShmID, NULL, 0);
  if (*ShmPTR == -1) {
    printf("*** shmat error (server) ***\n");
    exit(1);
  }

  ShmPTR[0] = bankAccount;

  // child and parent process computations

  if (fork() == 0) { /* child process*/
    for (int i = 0; i < 10 ; i++) {
      ChildProcess(ShmPTR);
    }
  }
  // parent process
  for (int i = 0; i < 10 ; i++){
    ParentProcess(ShmPTR);
  }
}

void ParentProcess(int SharedMem[]){
  sleep(rand() % 6);

  printf("Dear Old Dad: Attempting to Check Balance\n");

  sem_wait(mutex);
  int localBalance = SharedMem[0];

  int randomNum = rand(); // make random later

  if (randomNum % 2 == 0){
    if (localBalance < 100){
      depositMoney(SharedMem);
    }
    else{
      printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", localBalance);
    }
  }
  else{
    printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
  }
  sem_post(mutex);
}

void  depositMoney(int SharedMem[]){
  int localBalance = SharedMem[0];

  int amountGiven = rand() % 101;

  if (amountGiven % 2 == 0){
    localBalance += amountGiven;
    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", amountGiven, localBalance);
  }
  else{
    printf("Dear old Dad: Doesn't have any money to give\n");
  }

  SharedMem[0] = localBalance;
}

void  ChildProcess(int SharedMem[]){ 
  sleep(rand() % 6);
  printf("Poor Student attempting to check balance\n");

  sem_wait(mutex);

  int localBalance = SharedMem[0];

  // generate random number between 0 - 100
  int randomNum = rand(); // make random later

  // if random number is even withdraw money
  if (randomNum % 2 == 0){
    int amountStudentNeed = rand() % 51; 
    printf("Poor Student needs $%d\n", amountStudentNeed);

    if (amountStudentNeed <= localBalance){
      localBalance -= amountStudentNeed;
      printf("Poor Student: Withdraws $%d / Balance = $%d\n", amountStudentNeed, localBalance);
    }
    else{
      printf("Poor Student: Not Enough Cash ($%d)\n", localBalance);
    }

    SharedMem[0] = localBalance;
  }
  else{
    printf("Poor Student: Last Checking Balance = $%d\n", localBalance);
  }
  sem_post(mutex);
}

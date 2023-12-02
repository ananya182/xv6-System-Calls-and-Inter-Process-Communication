#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

#define buffersize 50

int senderarray[buffersize] = {0};
int recvrarray[buffersize] = {0};
char messagearray[buffersize][8];
int busy = 0;

int sys_fork(void)
{
  return fork();
}

int sys_exit(void)
{
  exit();
  return 0; // not reached
}

int sys_wait(void)
{
  return wait();
}

int sys_kill(void)
{
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int sys_getpid(void)
{
  return myproc()->pid;
}

int sys_sbrk(void)
{
  int addr;
  int n;

  if (argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

int sys_sleep(void)
{
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (myproc()->killed)
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_print_count(void)
{
  return print_count();
}

int sys_add(void)
{
  int n1, n2;
  argint(0, &n1);
  argint(1, &n2);
  return n1 + n2;
}

int sys_ps(void)
{
  return ps();
}

int sys_send(void)
{
  while (busy)
  {
    ;
  }

  busy = 1;
  int n1, n2;
  void *n3;

  argint(0, &n1);
  argint(1, &n2);
  argptr(2, (void *)&n3, 8);

  char *message = (char *)n3;

  for (int i = 0; i < buffersize; i++)
  {
    if (senderarray[i] == 0)
    {
      senderarray[i] = n1;
      for (int j = 0; j < 8; j++)
        messagearray[i][j] = message[j];
      recvrarray[i] = n2;
      busy = 0;
      return 0;
    }
  }
  return 1;
}

int sys_recv(void)
{
  void *n1;
  argptr(0, (void *)&n1, 8);

  int recvrpid = myproc()->pid;
  char *message = (char *)n1;

  while (1)
  {

    for (int i = 0; i < buffersize; i++)
    {
      if (recvrarray[i] == recvrpid)
      {
        for (int j = 0; j < 8; j++)
          message[j] = messagearray[i][j];

        recvrarray[i] = 0;
        senderarray[i] = 0;

        return 0;
      }
    }
  }
}

int sys_toggle(void)
{
  return 1;
}

int sys_send_multi(void)
{
  while (busy)
  {
    ;
  }

  busy = 1;
  int n1;
  void *n2, *n3;

  argint(0, &n1);
  argptr(1, (void *)&n2, 50);
  argptr(2, (void *)&n3, 8);

  int *recpids = (int *)n2;
  char *message = (char *)n3;
  int k = 0;

  for (int i = 0; i < buffersize; i++)
  {
    if (senderarray[i] == 0)
    {
      senderarray[i] = n1;
      for (int j = 0; j < 8; j++)
        messagearray[i][j] = message[j];
      recvrarray[i] = recpids[k];
      k++;
    }
    if (recpids[k] <= 0)
    {
      busy = 0;
      return 0;
    }
  }
  return 1;
}
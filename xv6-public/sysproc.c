#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;
	
// static struct proc *initproc;
	
int
sys_getpcount(void)
{
  struct proc *p;
//  char *sp;

  acquire(&ptable.lock);
  int count = 0;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    // UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE
    if(p->state == UNUSED){cprintf("UNUSED\n");}
    else if(p->state == EMBRYO){cprintf("EMBRYO\n");}
    else if(p->state == SLEEPING){cprintf("SLEEPING\n");}
    else if(p->state == RUNNABLE){cprintf("RUNNABLE\n");}
    else if(p->state == RUNNING){cprintf("RUNNING\n");}
    else if(p->state == ZOMBIE){cprintf("ZOMBIE\n");}
    else {cprintf("NONE\n");}
    cprintf("proc: %s\nstatus: %s\n\n", p->name, p->state);
    count++;
  }
  release(&ptable.lock);

//  printf(1, "Final Count: %d\n", count);
  return count;
}

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
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
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

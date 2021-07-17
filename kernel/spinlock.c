#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "arm.h"
#include "proc.h"
#include "defs.h"

void 
initlock(struct spinlock* lk, char* name)
{
    lk->name = name;
    lk->locked = 0;
    lk->cpu = 0;
};

//Acquire the lock
//Loops until the lock is acquired.
void
acquire(struct spinlock* lk)
{

}

//Release the lock.
void
release(struct spinlock* lk)
{

}

void 
push_off(void)
{

}

void 
pop_off(void)
{
    
}
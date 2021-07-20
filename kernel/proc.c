#include "types.h"
#include "param.h"
#include "defs.h"
#include "spinlock.h"
#include "proc.h"

struct cpu CPUS[NCPU];
struct cpu* cpu;

struct proc PROCS[NPROC];
struct proc* initproc;
struct proc* proc;

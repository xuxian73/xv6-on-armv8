struct sleeplock {
    uint locked;
    struct spinlock lk;
    char * name;
    int pid;
};
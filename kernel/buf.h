struct buf {
    int valid;  // has data been read from disk?
    int disk;   // does disk "own" buf?
    uint dev;
    uint blockno;
    struct sleeplock lock;
    uint refcnt;
    struct buf *prev;   // LRU cache list
    struct buf *next;
    uchar  data[BSIZE];
};

#define B_BUSY  0x1  // buffer is locked by some process
#define B_VALID 0x2  // buffer has been read from disk
#define B_DIRTY 0x4  // buffer needs to be written to disk
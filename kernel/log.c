#include "types.h"
#include "arm.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

// Simple logging that allows concurrent FS system calls;
//
// A log transaction contains the updates of multiple FS system 
// calls. The loggint system only commits when there are
// no FS system calls active. Thus there is never
// any reasoning required about whether a commit might
// write an uncommitted system call's updates to disk.
//
// A system call should call begin_op()/end_op() to mark 
// its start and end. Usually begin_op() just increments
// the count of in-progresss FS system calls and returns.
// But if it thinks the log is close to running out, it 
// sleeps until the last outstanding end_op() commits.
//
// The log is a physical re-do log containing disk blocks
// The on-disk log format:
//  header block, containing block #s for block A, B, C, ...
//  block A
//  block B
//  block C
//  ...
//  Log appends are synchronus.

// Contents of the header block, used for both the on-disk header block
// and to keep track in memeory of logged block# before commit.
struct logheader {
    int n;
    int block[LOGSIZE];
};

struct log {
    struct spinlock lock;
    int start;
    int size;
    int outstanding; // how many FS sys calls are executing.
    int committing; // in commit(), please wait
    int dev;
    struct logheader lh;
};

struct log Log;

static void recover_from_log(void);
static void commit();

void initlog(int dev, struct superblock *sb)
{
    if(sizeof(struct logheader) >= BSIZE)
        panic("initlog: too big logheader");
    
    initlock(&Log.lock, "log");
    Log.start = sb->logstart;
    Log.size = sb->nlog;
    Log.dev = dev;
    recover_from_log();
}

// Copy committed blocks from log to their home location
static void
install_trans(int recovering)
{
    int tail;
    for (tail = 0; tail < Log.lh.n; ++tail) {
        struct buf *lbuf = bread(Log.dev, Log.start + tail + 1);    // read log block
        struct buf *dbuf = bread(Log.dev, Log.lh.block[tail]);      // read dst
        memmove(dbuf->data, lbuf->data, BSIZE);
        bwrite(dbuf);
        if(recovering == 0)
            bunpin(dbuf);
        brelse(lbuf);
        brelse(dbuf);
    }
}

// Read the log header from disk into the in-memory log header
static void
read_head(void)
{
    struct buf *buf = bread(Log.dev, Log.start);
    struct logheader *lh = (struct logheader*) (buf->data);
    int i;
    Log.lh.n = lh->n;
    for(i = 0; i < Log.lh.n; ++i) {
        Log.lh.block[i] = lh->block[i];
    }
    brelse(buf);
}

// Write in-memory log header to disk.
// This is the true point at which the 
// current transaction commits.
static void
write_head(void)
{
    struct buf *buf = bread(Log.dev, Log.start);
    struct logheader *hb = (struct logheader *)(buf->data);
    int i;
    hb->n = Log.lh.n;
    for (i = 0; i < Log.lh.n; ++i) {
        hb->block[i] = Log.lh.block[i];
    }
    bwrite(buf);
    brelse(buf);
}

static void
recover_from_log(void)
{
    read_head();
    install_trans(1);   // if committed, copy from log to dist
    Log.lh.n = 0;
    write_head();
}

// called at the start of each FS system call.
void
begin_op(void)
{
    acquire(&Log.lock);
    while(1) {
        if(Log.committing) {
            sleep(&Log, &Log.lock);
        } else if(Log.lh.n + (Log.outstanding+1)*MAXOPBLOCKS > LOGSIZE) {
            // this op might exhaust log space; wait for commit.
            sleep(&Log, &Log.lock);
        } else {
            Log.outstanding += 1;
            release(&Log.lock);
            break;
        }
    }
}

// called at the end of each FS system call.
// commits if this was the last outstanding operation.
void
end_op(void)
{
    int do_commit = 0;
    acquire(&Log.lock);
    Log.outstanding -= 1;
    if(Log.committing)
        panic("log.committing");
    if(Log.outstanding == 0) {
        do_commit = 1;
        Log.committing = 1;
    } else {
        wakeup(&Log);
    }
    release(&Log.lock);

    if(do_commit) {
        commit();
        acquire(&Log.lock);
        Log.committing = 0;
        wakeup(&Log);
        release(&Log.lock);
    }
}

// Copy modified blocks from cache to log
static void
write_log(void)
{
    int tail;
    for(tail = 0; tail < Log.lh.n; ++tail) {
        struct buf *to = bread(Log.dev, Log.start + tail + 1);
        struct buf *from = bread(Log.dev, Log.lh.block[tail]);
        memmove(to->data, from->data, BSIZE);
        bwrite(to);
        brelse(from);
        brelse(to);
    }
}

static void
commit()
{
    if(Log.lh.n > 0) {
        write_log();    // Write modified blocks form cache to log
        write_head();   // Write header to disk -- the real commit
        install_trans(0); //Now install write to home locations
        Log.lh.n = 0;
        write_head();   // Erase the trasaction from the log
    }
}

// Caller has modified b->data and is done with the buffer
// Record the block number and pin in the cache by increasing refcnt.
// commit()/write_log() will do the disk write
//
// log_write() replaces bwrite(); a typical use is:
// bp = bread(...)
// modify bp->data[]
// log_write(bp)
// brelse(bp)
void
log_write(struct buf *b)
{
    int i;

    if (Log.lh.n >= LOGSIZE || Log.lh.n >= Log.size - 1)
        panic("too big a transaction");
    if(Log.outstanding < 1)
        panic("log_write outside of trans");
    
    acquire(&Log.lock);
    for(i = 0; i < Log.lh.n; ++i) {
        if(Log.lh.block[i] == b->blockno)
            break;
    }
    Log.lh.block[i] = b->blockno;
    if (i == Log.lh.n) {
        bpin(b);
        Log.lh.n++;
    }
    release(&Log.lock);
}
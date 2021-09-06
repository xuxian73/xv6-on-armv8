// On-disk file system format.
// Both the kernel and user programs use this header file.

// Block 0 is unused.
// Block 1 is super block.
// Blocks 2 through sb.ninodes/IPB hold inodes.
// Then free bitmap blocks holding sb.size bits.
// Then sb.nblocks data blocks.
// Then sb.nlog log blocks.

#define ROOTINO 1   // root i-number
#define BSIZE 1024   // block size

// File system super block
struct superblock {
    uint magic;
    uint size;      // Size of file system image (blocks)
    uint nblocks;   // Number of data blocks
    uint ninodes;   // Number of inodes.
    uint nlog;      // Number of log blocks
    uint logstart;
    uint inodestart;
    uint bmapstart;
};

#define FSMAGIC 0x10203040
#define NDIRECT 12
#define NINDIRECT (BSIZE /sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure
struct dinode {
    short   type;           // File type
    short   major;          // Major device number (T_DEV only)
    short   minor;          // Minor device number (T_DEV only)
    short   nlink;          // Number of links to inode in file system
    uint    size;           // Size of file (bytes)
    uint    addrs[NDIRECT+1]; // Data block addresses
};

// Inodes per block
#define IPB     (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i, sb)   ((i) / IPB + sb.inodestart)

// Bitmap bits per block
#define BPB     (BSIZE * 8)

// Block of free map containig bit for block b
#define BBLOCK(b, sb)   ((b)/BPB + sb.bmapstart)

//Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
    ushort inum;
    char name[DIRSIZ];
};
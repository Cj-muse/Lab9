#ifndef HEADER_H
#define HEADER_H

//#include <math.h>

#define NPROC    9
#define SSIZE 1024
#define BLOCK_SIZE        1024
#define BLKSIZE           1024

// Block number of EXT2 FS on FD
#define SUPERBLOCK        1
#define GDBLOCK           2
#define BBITMAP           3
#define IBITMAP           4
#define INODEBLOCK        5
#define ROOT_INODE        2

// Default dir and regulsr file modes
#define DIR_MODE          0040777
#define FILE_MODE         0100644
#define SUPER_MAGIC       0xEF53
#define SUPER_USER        0

/******* PROC status ********/
#define FREE     0
#define READY    1
#define RUNNING  2
#define STOPPED  3
#define SLEEP    4
#define ZOMBIE   5

/**********For Pipes***********/
#define READ_PIPE  4
#define WRITE_PIPE 5
#define NOFT      20
#define NFD       20
#define PSIZE     10
#define NPIPE     10

/******** For Semaphores *********/
#define BLOCK    6


extern int color;
//extern int MTXSEG  = 0x1000;

typedef unsigned char  u8;
typedef unsigned int  u16;
typedef unsigned long  u32;

#define  GREEN  10         // color byte for putc()
#define  CYAN   11
#define  RED    12
#define  MAG    13
#define  YELLOW 14

// io globals
int BASE = 10;
int *FP;
char *table = "0123456789ABCDEF";

#define  BOOTSEG 0x9000
#define MAX 256
#define NULL 0
char mbr[512];
char ans[64];
char buffer[1024];
char buffer2[1024];
char buffer3[1024];
u16 InodeBeginBlk;
////////////////////////////////////////////////////////////////////////////
// inode struct definitions
////////////////////////////////////////////////////////////////////////////

typedef struct ext2_super_block {
  u32	s_inodes_count;		/* Inodes count */
  u32	s_blocks_count;		/* Blocks count */
  u32	s_r_blocks_count;	/* Reserved blocks count */
  u32	s_free_blocks_count;	/* Free blocks count */
  u32	s_free_inodes_count;	/* Free inodes count */
  u32	s_first_data_block;	/* First Data Block */
  u32	s_log_block_size;	/* Block size */
  u32	s_log_frag_size;	/* Fragment size */
  u32	s_blocks_per_group;	/* # Blocks per group */
  u32	s_frags_per_group;	/* # Fragments per group */
  u32	s_inodes_per_group;	/* # Inodes per group */
  u32	s_mtime;		/* Mount time */
  u32	s_wtime;		/* Write time */
  u16	s_mnt_count;		/* Mount count */
  u16	s_max_mnt_count;	/* Maximal mount count */
  u16	s_magic;		/* Magic signature */
  u16	s_state;		/* File system state */
  u16	s_errors;		/* Behaviour when detecting errors */
  u16	s_minor_rev_level; 	/* minor revision level */
  u32	s_lastcheck;		/* time of last check */
  u32	s_checkinterval;	/* max. time between checks */
  u32	s_creator_os;		/* OS */
  u32	s_rev_level;		/* Revision level */
  u16	s_def_resuid;		/* Default uid for reserved blocks */
  u16	s_def_resgid;		/* Default gid for reserved blocks */

  // these are for DYNAMIC ext2_fs used in 2.6 kernel
  u32   s_first_ino;
  u16   s_inode_size;           /* real inode size : OLD=128, NEW=256*/
  u16	s_block_group_nr;	/* block group # of this superblock */
  u32	s_feature_compat;	/* compatible feature set */
  u32	s_feature_incompat;	/* incompatible feature set */
  u32	s_feature_ro_compat;	/* readonly-compatible feature set */
  u8	s_uuid[16];		/* 128-bit uuid for volume */
  char	s_volume_name[16];	/* volume name */
  char	s_last_mounted[64];	/* directory where last mounted */
  u32	s_algorithm_usage_bitmap; /* For compression */

  /**** ignore the rest; not needed for booting **********/
} SUPER;

typedef struct ext2_group_desc
{
  u32  bg_block_bitmap;
  u32  bg_inode_bitmap;
  u32  bg_inode_table;
  u16  bg_free_blocks_count;
  u16  bg_free_inodes_count;
  u16  bg_used_dirs_count;
  u16  bg_pad;
  u32  bg_reserved[3];
} GD;

typedef struct ext2_inode {
  u16	i_mode;		/* File mode */
  u16	i_uid;		/* Owner Uid */
  u32	i_size;		/* Size in bytes */
  u32	i_atime;	/* Access time */
  u32	i_ctime;	/* Creation time */
  u32	i_mtime;	/* Modification time */
  u32	i_dtime;	/* Deletion Time */
  u16	i_gid;		/* Group Id */
  u16	i_links_count;	/* Links count */
  u32	i_blocks;	/* Blocks count */
  u32	i_flags;	/* File flags */
  u32   dummy;
  u32	i_block[15];    /* Pointers to blocks */
  u32   pad[7];         /* inode size MUST be 128 bytes */
} INODE;

typedef struct ext2_dir_entry_2 {
  u32	inode;			/* Inode number */
  u16	rec_len;		/* Directory entry length */
  u8	name_len;		/* Name length */
  u8	file_type;
  char name[255];      	/* File name */
} DIR;

typedef struct header{
     u32 ID_space;         // 0x04100301:combined I&D or 0x04200301:separate I&D
     u32 magic_number;     // 0x00000020
     u32 tsize;            // code section size in bytes
     u32 dsize;            // initialized data section size in bytes
     u32 bsize;            // bss section size in bytes
     u32 zero;             // 0
     u32 total_size;       // total memory size, including heap
     u32 symbolTable_size; // only if symbol table is present
} HEADER;

////////////////////////////////////////
// globals
///////////////////////////////////////
GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dir;
char  *cp;

//pipe structs
typedef struct pipe{
  char  buf[PSIZE];
  int   head, tail, data, room;
  int   nreader, nwriter;
  int   busy;
}PIPE;

PIPE pipe[NPIPE];
#define BROKEN_PIPE 24

typedef struct Oft{
  int   mode;
  int   refCount;
  struct pipe *pipe_ptr;
} OFT;

OFT  oft[NOFT];

typedef struct proc{
    struct proc *next;
    int    *ksp;               // at offset 2

    int    uss, usp;           // at offsets 4,6
    int    inkmode;            // at offset 8

    int    pid;                // add pid for identify the proc
    int    status;             // status = FREE|READY|RUNNING|SLEEP|ZOMBIE
    int    ppid;               // parent pid
    struct proc *parent;
    
    struct semaphore *sem;     // pointer to SEMPAPHORE if Proce is blocked
    
    int    priority;
    int    event;
	 int    sleeptime;
	 int    runtime;
    int    exitCode;
    char   name[32];           // name string of PROC

    OFT    *fd[NFD];

    int    kstack[SSIZE];      // per proc stack area
}PROC;

typedef struct semaphore{
   int  lock;      // spinlock
   int  value;
   PROC *queue;      /* a FIFO queue */
} SEMAPHORE;


struct partition {         // Partition table entry in MBR
       u8  drive;          // 0x80 - active
       u8  head;     // starting head
       u8  sector;      // starting sector
       u8  cylinder;       // starting cylinder
       u8  type;     // partition type
       u8  end_head;       // end head
       u8  end_sector;     // end sector
       u8  end_cylinder;   // end cylinder
       u32 start_sector;   // starting sector counting from 0
       u32 nr_sectors;     // nr of sectors in partition
};

struct dap{                // DAP for extended INT 13-42
       u8   len;           // dap length=0x10 (16 bytes)
       u8   zero;          // must be 0
       u16  nsector;       // number of sectors to read: 1 to 127
       u16  addr;          // memory address = (segment:addr)
       u16  segment;
       u32  s1;            // low  4 bytes of sector#
       u32  s2;            // high 4 bytes of sector#
};

struct dap dap, *dp;       // global dap struct


/****** type.h additions *********/
PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC);
int nproc = 0;

//int goUmode();
int body();
char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter",
               "Saturn", "Uranus", "Neptune" };

/****** function headers *******/
//ts.s
int goUmode();

// t.c
int body();
int init();
int scheduler();
PROC *kfork();

// commands.c
do_tswitch();
do_kfork();
do_ps();
do_exit();
do_sleep();
do_wake();
do_wait();

// kernel.c
int ksleep(int event);
int kwakeup(int event);
int kexit(int exitvalue);
int kwait(int *status);

PROC *kfork(char *filename);
int copyFds(PROC *p);
int kgetpid();
int kprintstatus();
int kchname(char name[32]);
int kmode();
int get_block(u16 blk, char *buf);

//inode.c
int getInodeFromFile(char *filename);
int chopFirstStringElement(char path[10][32]);
int findInode(char path[10][32]);
u16 search(char name[32]);
INODE *getINODE(int ino);
int parseInput(char *input, char **parsedinput, char *delimiter);

//ForkExec.c
//int clear_bss(u16 segment, u16 tsize, u16 dsize, u16 bsize);
//int move(segment, tsize, dsize) u16 segment, tsize, dsize;
int fork();
int exec();
int ufork();
int uexec();

//pipe.c
show_pipe(PIPE *p);
int pfd();
int read_pipe(int fd, char *buf, int n);
int write_pipe(int fd, char *buf, int n);
int kpipe(int pd[2]);
int close_pipe(int fd);
PIPE *initPipe();
OFT *initOFT(int mode, PIPE *p);

//io.c
int rpu(u32 x);
int rpu16(u16 x);
int printu(u32 x);
int printd(int x);
int printo(u32 x);
int printx(u32 x);
void printf(char *fmt, ...);
int prints(char *s);
strcmp();
int gets(char s[64]);
int strtoint(char *str);
int isDigit(char c);
int power(int x, int y);

// queue
void showLists();

#endif

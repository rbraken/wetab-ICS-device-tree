// 02-2012 Hack by RvdB to use shm on android
// Created from different sources for 32 bit android kernel
// N.B. kernel need shm ofcoarse
// 
#define IPC_CREAT  01000
#define IPC_EXCL   02000
#define IPC_NOWAIT 04000
#define IPC_RMID 0
#define IPC_SET  1
#define IPC_STAT 2
#define IPC_INFO 3

#define IPC_PRIVATE ((key_t) 0)

struct ipc_perm
{
	key_t key;
	uid_t uid;
	gid_t gid;
	uid_t cuid;
	gid_t cgid;
	mode_t mode;
	int seq;
	long __pad1;
	long __pad2;
};
#define SHMLBA 4096

#define SHM_RDONLY 010000
#define SHM_RND    020000
#define SHM_REMAP  040000
#define SHM_EXEC   0100000

/* linux extensions */
#define SHM_LOCK        11
#define SHM_UNLOCK      12

typedef unsigned long int shmatt_t;
/* Data structure describing a shared memory segment.  */
struct shmid_ds
  {
    struct ipc_perm shm_perm;           /* operation permission struct */
    size_t shm_segsz;                   /* size of segment in bytes */
    __time_t shm_atime;                 /* time of last shmat() */
    unsigned long int __unused1;
    __time_t shm_dtime;                 /* time of last shmdt() */
    unsigned long int __unused2;
    __time_t shm_ctime;                 /* time of last change by shmctl() */
    unsigned long int __unused3;
    __pid_t shm_cpid;                   /* pid of creator */
    __pid_t shm_lpid;                   /* pid of last shmop */
    shmatt_t shm_nattch;                /* number of current attaches */
    unsigned long int __unused4;
    unsigned long int __unused5;
  };

__BEGIN_DECLS

extern void*  shmat(int  shmid, const void*  shmaddr, int  shmflg);
extern int    shmctl(int  shmid, int  cmd, struct shmid_ds*  buf);
extern int    shmdt(const void*  shmaddr);
extern int    shmget(key_t  key, size_t  size, int  shmflg);

__END_DECLS



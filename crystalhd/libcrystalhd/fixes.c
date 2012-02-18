#include <utils/Log.h>
#undef LOG_TAG
#define LOG_TAG "Fixes"
#include <errno.h>
#include <stdlib.h>		/* for definition of NULL */
#include <sys/syscall.h>		/* for definition of NULL */
#include "shm.h"		/* for definition of NULL */

#define IPCOP_shmat     21
#define IPCOP_shmdt     22
#define IPCOP_shmget    23
#define IPCOP_shmctl    24

#define IPC_MODERN   0x100
#define SYS_ipc  117


int shmget(key_t key, size_t size, int flag)
{
        int rc;
        LOGV("get flag=%x\n", flag);
        rc= syscall (117, IPCOP_shmget, key, size, flag, NULL);
        LOGV("get rc  =%x errno=%d\n",rc,errno);
	return rc ;
}

int shmdt(const void *addr)
{
  int rc ;
        LOGV("dt addr=%p\n", addr);
        rc= syscall(SYS_ipc, IPCOP_shmdt, 0,0,0,addr);
        LOGV("dt rc  =%x errno=%d\n",rc,errno);
	return rc ;
}

int shmctl(int id, int cmd, struct shmid_ds *buf)
{
       int rc; 
       LOGV("ctl id=%d cmd=%x buf=%p\n", id,cmd,buf);
       rc= syscall(SYS_ipc, IPCOP_shmctl, id, cmd | IPC_MODERN, 0,buf);
       LOGV("ctl rc=%d\n",rc);
       return rc ;
}

void *shmat(int id, const void *addr, int flag)
{
        void *rc;
        unsigned long ret;
        LOGV("mat id=%d addr=%p\n", id,addr);
        ret = syscall(SYS_ipc, IPCOP_shmat, id, flag, &addr, addr);
        LOGV("mat ret=%lx\n", ret);
        rc= (ret > -(unsigned long)SHMLBA) ? (void *)ret : (void *)addr;
        LOGV("mat id=%d addr=%p rc=%p\n", id,addr,rc);
	return rc ;
}

/* define the missing posix_memalign using android memalign */

int posix_memalign(void **res, size_t align, size_t len)
{
        LOGV("posix len=%d\n", len);
        void *ptr = NULL;
        ptr =  memalign(align,len) ;
	if (ptr == NULL) return ENOMEM ;
	*res = ptr ;
	return 0 ;
}


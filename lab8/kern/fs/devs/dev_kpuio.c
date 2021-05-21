#include <defs.h>
#include <stdio.h>
#include <wait.h>
#include <sync.h>
#include <proc.h>
#include <sched.h>
#include <dev.h>
#include <vfs.h>
#include <iobuf.h>
#include <inode.h>
#include <unistd.h>
#include <error.h>
#include <assert.h>
#include <sbi.h>
#include <io.h>
#include <trap.h>
#include <cnn.h>
#include <stdio.h>

static int
kpuio_open(struct device *dev, uint32_t open_flags)
{
    if (open_flags != O_RDWR)
    {
        return -E_INVAL;
    }
    return 0;
}

static int
kpuio_close(struct device *dev)
{
    return 0;
}
#include<cnn.h>
kpu_buff* kputaskbase, *kpuresultbase;
int caller_pid;
char buffer[4096];

bool kpuio_init;      //init action
bool kpuio_check;     //get result action
//kpu_intr handler
//  1. get running task, 省略一致性检查了，默认我的调度器不会有bug
//  2. mark running task as stoped
//   
int 
dev_kpuio_intr(void){
    return 0;
}

int totlength, sumlen;
void* bufs[64];
int lens[64];
int numblock;

int
dev_kpuio_taskinit(void *buf, size_t len, int pid, bool first, int totsize){
    int ret = 0;
    bool intr_flag;
    local_intr_save(intr_flag);
    {
        //for(int i = 0; i < len; i++)
        cprintf("[dev_kpuio_taskinit]current pid %d\n", current->pid);
        // if(len != 1){
        //     panic("kpuio: task num > 1 error\n");
        //     return -E_INVAL;
        // }
        if (first) {
            numblock = 0;
            totlength = totsize;
            sumlen = 0;
            if (totlength < 0) {
                warn("[dev_kpuio_taskinit]totlen < 0 error\n");
                return -1;
            }
            caller_pid = pid;
        }
        
        if (!first){
            if (caller_pid != pid){
                panic("[dev_kpuio_taskinit]被多个进程同时init污染了\n");
                return -1;
            }
        }
  
        cprintf("[dev_kpuio_taskinit]totlength is %d ; buf is %p\n",totlength, buf);
        //do the copy stuff
        lens[numblock] = len;
        bufs[numblock] = kmalloc(len);
        for(int i = 0; i < len; i++){
            ((char*)bufs[numblock])[i] = *((char*)buf + i);
        }
        numblock++;
        sumlen += len;
        //kputaskbase->jpeg = (char *)&buffer[0];
        //打印出来
    }

    if(sumlen >= totlength){
        kpuio_init = true; kpuio_check = false;
        cprintf("[dev_kpuio_taskinit]run_kpu_task_add\n");
        run_kpu_task_add();
        //reset mark
        kpuio_init = kpuio_check = false;        
    }
    
    local_intr_restore(intr_flag);
    return ret;
}

int
dev_try_getresult(void* buf, size_t len, int pid){
    int ret = 0;
    caller_pid = pid;
    cprintf("[dev_try_getresult] init %d, check %d \n",kpuio_init,kpuio_check);
    kpuresultbase = (kpu_buff*)kmalloc(4096);
    run_kpu_task_check(buf , len, pid);
    //reset mark
    cprintf("[dev_try_getresult] got status %d\n", kpuresultbase->status);
    {
        //copy to buf so that user can receive
        ((kpu_buff*)buf)->status = kpuresultbase->status;
    }
    kpuio_init = kpuio_check = false;
    kfree(kpuresultbase);
    return ret;
}

// current proc
extern struct proc_struct *current;
void *lastbuf = NULL;
static int
kpuio_io(struct device *dev, struct iobuf *iob, bool write)
{
    if (lastbuf != (void *)iob)
        lastbuf = iob;
    //else return 0;
    if (write)
    {
        int ret;
        kpuio_init = true; kpuio_check = false;
        cprintf(" kpuio_io init %d, check %d current pid %d\n",kpuio_init,kpuio_check, current->pid);
        panic("cannot use \n");
        //ret = dev_kpuio_taskinit(iob->io_base, iob->io_resid, current->pid);
        return ret;
    }else{
        kpuio_check = true; kpuio_init = false;
        cprintf("[kpuio_read] init %d, check %d iob->base %p\n",kpuio_init,kpuio_check, iob->io_base);
        int ret = dev_try_getresult(iob->io_base, iob->io_resid, current->pid);
        cprintf("[kpuio_io]got result %d\n", ((kpu_buff*)iob->io_base)->status);
        return ret;
    }
    return -E_INVAL;
}

static int
kpuio_ioctl(struct device *dev, int op, void *data)
{
    return -E_UNIMP;
}

static void
kpuio_device_init(struct device *dev)
{
    dev->d_blocks = 0;
    dev->d_blocksize = 1;
    dev->d_open = kpuio_open;
    dev->d_close = kpuio_close;
    dev->d_io = kpuio_io;
    dev->d_ioctl = kpuio_ioctl;
}

void dev_init_kpuio(void)
{
    struct inode *node;
    if ((node = dev_create_inode()) == NULL)
    {
        panic("stdin: dev_create_node.\n");
    }
    kpuio_device_init(vop_info(node, device));

    int ret;
    if ((ret = vfs_add_dev("kpuio", node, 0)) != 0)
    {
        panic("kpuio: vfs_add_dev: %e.\n", ret);
    }

    kpuio_init = kpuio_check = false;
}
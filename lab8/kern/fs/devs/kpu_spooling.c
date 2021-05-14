
#include <kpu_spooling.h>


#include <kmalloc.h>
#include <stdio.h>
#include <wait.h>
#include <sem.h>
#include <proc.h>
list_entry_t kpu_tasklist;
//暂时累计
int maxid;
static semaphore_t kpu_sem;

static int run_kpu(_kpu_pool_task_t* task){
    down(&kpu_sem);

    cprintf("[thread %d]start running task %d\n" ,current->pid, task->id);
    run_task(task);


    do_sleep(1000);


    stop_task(task);
    task_success(task);
    cprintf("return from run kpu, run success\n");

    up(&kpu_sem);
}

int add_runtest_thread(_kpu_pool_task_t* task){
    int pid = kernel_thread(run_kpu, (void*)task, 0);
    if(pid > 2) {
        cprintf("created kpu thread pid %d\n", pid);
        return 0;
    }
    return -1;
}


int try_check_result(int pid){
    list_entry_t* e = list_next(&kpu_tasklist);
    _kpu_pool_task_t* waitingtask = NULL;
    if (list_empty(&kpu_tasklist)){
        warn("empty list\n");
    }
    //check if its running
    while (e != &kpu_tasklist){
        _kpu_pool_task_t* task = le2task(e, task_link);
        if(!is_running(task) && is_success(task)){
            return RESULT_GOT;
        }
        else if(is_running(task)) {
            if(pid == task->proc->pid){
                return RESULT_RUNNING;
            }
        }
        else{
            if (pid == task->proc->pid){
                return RESULT_WAITING;
            }
        }
        e = list_next(e);
    }

    return RESULT_NOTEXIST;
}

/**
 * try to run a new task
 * return -1 if nothing is ready for run
 * return 0 if kpu is busy
 * return pid if kpu starts to run pid's task
 */

extern void kpu_test(_kpu_pool_task_t *runtask);
int try_run_task(int taskid){
    list_entry_t* e = list_next(&kpu_tasklist);
    _kpu_pool_task_t* runtask = NULL;
    while (e != &kpu_tasklist){
        _kpu_pool_task_t* task = le2task(e, task_link);
        if(is_running(task)) {
            warn("trying to run a running task\n");
            break;
        }else{
            if(task->id == taskid){
                runtask = task;
                break;
            }
        }
        e = list_next(e);
    } 
    if(runtask == NULL) return -1;

    //runtask
    cprintf("deliver task from pid%d, jpeg %p, size %d to kpu\n " 
            ,runtask->proc->pid, runtask->input->jpeg, runtask->input->jpgsize);
    add_runtest_thread(runtask);
    // 执行kpu_test(runtask);
    // kpu_test(runtask);
    return runtask->proc->pid;
}
static int alloc_id(){
    maxid = (maxid + 1) % MAX_TASKNUM;
    return maxid;
}
// add new task to list
// return taskid, if -1 failed
extern int totlength;
extern void* bufs[64];
extern int lens[64];
extern int numblock;
int add_kpu_task(int callerpid){
    assert(totlength > 0);
    kpu_buff* buff = kmalloc(totlength);
    cprintf("buff's totsize is %d\n", totlength);
    char* dst_ptr, *src_ptr;
    dst_ptr = (char*)buff;
    for(int i = 0; i < numblock; i++){
        char* src_ptr = (char*)bufs[i];
        cprintf("copying %d bytes from %p to %p\n", lens[i], src_ptr, dst_ptr);
        for(int j = 0; j < lens[i]; j++){
            *dst_ptr = *src_ptr;
            dst_ptr++;
            src_ptr++;
        }
        kfree(bufs[i]);
        lens[i] = 0;
    }
    if(buff->totsize != totlength){
        panic("tot_size doesn't matchd should be %d but %d\n", buff->totsize , totlength);
    }
    buff->jpeg = (char*) buff + (offsetof(kpu_buff, jpeg) + sizeof(uintptr_t));
    _kpu_pool_task_t* newtask;
    cprintf("[add_kpu_task], buff addr:%p,current pid:%d\n", buff,current->pid);
    if(buff->jpgsize > 0){
        newtask =  kmalloc(buff->totsize);
        newtask->id = alloc_id();
        if(newtask->id >= 0){
            newtask->input = buff;
            newtask->proc = find_proc(callerpid);

            list_add_before(&kpu_tasklist, &(newtask->task_link));
            cprintf("[add_kpu_task]taskid: %d, totsize %d\n",newtask->id,
            newtask->input->totsize);
            cprintf("[add_kpu_task]check jpg: ");
            for(int j = 0; j < newtask->input->jpgsize; j++){
                cprintf("%d:%c ",j, newtask->input->jpeg[j]);
            }
            task_init(newtask); //set flag
            cprintf("\n");
            cprintf("[add_kpu_task]jpeg addr:%p, \n", newtask->input->jpeg);
            return newtask->id;
        }
    }
    else{
        warn("add fail 99\n");
        return 99;
    }
    return 100;
FREE_TASK:
    kfree(newtask);    
    return -1;
}
 
void kpu_spooling_init(void){
    maxid = 0;
    list_init(&kpu_tasklist);
    sem_init(&kpu_sem,1);

    cprintf("kpu spooling init\n");
}
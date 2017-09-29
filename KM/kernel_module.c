#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/delay.h>	/* Needed for msleep function*/
#include <linux/init.h>		/* Needed for the macros */
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/kthread.h>  // for threads
#include <linux/time.h>   // for using jiffies 


MODULE_LICENSE("GPL");

void handler(void)
{
	msleep(500);
	printk(KERN_INFO "Entrei no handler!\n");
}

//////////////////////////////////////////////////////////
static struct task_struct *kthread2;

int threadfn(void *unused)
{
	unsigned long j0,j1;
	int delay = 5*HZ;
	j0 = jiffies;
	j1 = j0 + delay;

	printk(KERN_INFO "In thread2");

	while (time_before(jiffies, j1))
        schedule();

	handler();
	return 0;
}


//////////////////////////////////////////////////////////

static struct task_struct *kthread1;
/*
int threadfn(void *unused)
{
	handler();
	return 0;
}
*/
//////////////////////////////////////////////////////////

static struct timer_list exp_timer;

static void ktimer_handler(unsigned long data)
{
	handler();
}

//////////////////////////////////////////////////////////

static void scheduled_work_handler(struct work_struct *w);
static DECLARE_DELAYED_WORK(scheduled_work, scheduled_work_handler);

static void scheduled_work_handler(struct work_struct *w)
{
	handler();
}

//////////////////////////////////////////////////////////

static struct workqueue_struct *queue;

static void work_handler(struct work_struct *w)
{
	printk(KERN_INFO "Worker\n");
	handler();
}

static DECLARE_WORK(work, work_handler);

//////////////////////////////////////////////////////////

static void tasklet_handler(unsigned long flag);

DECLARE_TASKLET(tasklet, tasklet_handler, 0);

static void tasklet_handler(unsigned long flag)
{
	printk("tasklet running...\n");
	handler();
	return;
}

//////////////////////////////////////////////////////////

static int __init kernel_module_init(void)
{
/*
	printk("module init start. \n");
	tasklet_schedule(&tasklet);

	queue = create_workqueue("WorkQueue");
	queue_work(queue, &work);

*/	
	queue = create_workqueue("WorkQueue");
	schedule_delayed_work(&scheduled_work, 5 * HZ);
/*
	setup_timer(&exp_timer, ktimer_handler, 0);
	mod_timer(&exp_timer, jiffies + msecs_to_jiffies(5000));
	
	
	kthread1 = kthread_create(threadfn, NULL,"Kthread1");
	kthread_bind(kthread1,1); //WITHOUT THIS THE THREAD WILL RUN ON ANY CPU;


	if(kthread1)
		wake_up_process(kthread1);


	
	kthread2 = kthread_create(threadfn, NULL,"Kthread2");
	
if(kthread2)
		wake_up_process(kthread2);


*/
	return 0;
}

static void __exit kernel_module_exit(void)
{
/*
	tasklet_kill(&tasklet);

	
	flush_work(&work);
	destroy_workqueue(queue);

*/
	cancel_delayed_work(&scheduled_work);
	flush_scheduled_work();
	destroy_workqueue(queue);
/*
	del_timer(&exp_timer);
		
	if (kthread1)
	{
       kthread_stop(kthread1);
       printk(KERN_INFO "Thread stopped");
   	}
	
	if (kthread2)
	{
       kthread_stop(kthread2);
       printk(KERN_INFO "Thread stopped");
   	}
*/
	printk("module init end.\n");
}

module_init(kernel_module_init);
module_exit(kernel_module_exit);

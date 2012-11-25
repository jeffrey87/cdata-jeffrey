#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include "cdata_ioctl.h"

#ifdef CONFIG_SMP
#define __SMP__
#endif

#define	CDATA_MAJOR 121 
#define BUFSIZE 1024

#define WIDTH 240
#define HEIGHT 320
#define BPP	4

#define LCD_LENGTH (WIDTH * HEIGHT * BPP)

struct cdata_t {
	char data[BUFSIZE];
	int index;

	char *iomem;
	struct timer_list timer;

	wait_queue_head_t	wait; //exe7

};

static DECLARE_MUTEX(cdata_sem);

static int cdata_open(struct inode *inode, struct file *filp)
{
	/* no reentrancy  because P1 and P2 have their own malloc*/ 
	
	int minor;
	struct cdata_t *cdata;
	minor = MINOR(inode->i_rdev);
	/*
	printk(KERN_ALERT "cdata: in cdata_open(minor = %d)\n", minor);
	printk(KERN_ALERT "cdata: the file struct address *filp is %d\n", filp);
	*/

	cdata = (struct cdata_t *)kmalloc(sizeof(struct cdata_t), GFP_KERNEL);
	cdata->index = 0;  /* the kmalloc does not clear the memory vlaue */ 
	filp->private_data = (void *)cdata;
	init_waitqueue_head(&cdata->wait); //exe7


	cdata->iomem  = ioremap(0x33f00000, LCD_LENGTH);
	init_timer(&cdata->timer);


	return 0;
}


static int cdata_ioctl(struct inode *inode, struct file *filp, 
			unsigned int cmd, unsigned long arg)
{
	int i;
	struct cdata_t *cdata = (struct cdata_t *)filp->private_data;
	
	printk(KERN_ALERT "cdata: in cdata_ioctl()\n");
	switch(cmd) 
	{
		case CDATA_EMPTY:
			printk(KERN_ALERT "in ioctl: IOCTL_EMPTY\n");
			for (i = 0; i < BUFSIZE; i++)
				cdata->data[i] = ' ';				
		
			printk(KERN_ALERT "------clear data------\n");
			for (i = 0; i < BUFSIZE; i++)
				printk(KERN_ALERT "%c", cdata->data[i]);

			printk(KERN_ALERT "\n");

			break;

		case CDATA_SYNC:
			printk(KERN_ALERT "in inctl: IOCTL_SYNC\n");
			printk(KERN_ALERT "The input value is :\n");
			for (i = 0; i < cdata->index; i++)
				printk(KERN_ALERT "%c",cdata->data[i]);
			printk(KERN_ALERT "\n");

			break;

		default:
			return -ENOTTY;
	}

	return 0;
}


static ssize_t cdata_read(struct file *filp, char *buf, 
				size_t size, loff_t *off)
{
	printk(KERN_ALERT "cdata: in cdata_read()\n");

	return 0;
}


/* supporting APIs */
void flush_lcd(unsigned long priv)
{
	struct cdata_t *cdata = (struct cdata_t *)priv;
	char *fb = cdata->iomem;
	int index = cdata->index;
	int i;

	for (i = 0; i < index; i++)
	{
		writeb(cdata->data[i], fb++);
	}

	cdata->index = 0;

	// wake up process
	current->state = TASK_RUNNING;
}


static ssize_t cdata_write(struct file *filp, const char *buf, 
				size_t count, loff_t *off)
{
	
	/* may have reentrant if open before fork */

	struct cdata_t *cdata = (struct cdata_t *)filp->private_data;
	int i;
	DECLARE_WAITQUEUE(wait, current); //exe 7

	struct timer_list *flush_timer;

	printk(KERN_ALERT "cdata_write: %s\n", buf);
	/* if CPU is single it may not reentrant, if SMP, it may */

	/* mutex_lock */
	down(&cdata_sem); //exe8

	for (i = 0; i < count; i++) 
	{

		if (cdata->index >= BUFSIZE) {
			
			add_wait_queue(&cdata->wait, &wait);// exe 7 = sleep_on
			set_current_state(TASK_UNINTERRUPTIBLE);

			// exe 7
			//current->state = TASK_UNINTERRUPTIBLE;
		
			flush_timer = &cdata->timer;
			flush_timer->expires = jiffies + 500;
			flush_timer->function = flush_lcd;
			flush_timer->data = (unsigned long) cdata;
			add_timer(flush_timer);

			up(&cdata_sem); //exe9
			schedule();
            down(&cdata_sem); //exe9

			/* current->state = TASK_RUNNING;  it is wrong concept */
			current->state = TASK_RUNNING;
			remove_wait_queue(&cdata->wait, &wait);
			
			del_timer(flush_timer);
		}

		
		/* if function have let process sleep like kmalloc,vmalloc, copy_from_user...  it must reentrant whather single or SMP */
		/* synchronization (share data) problem */

		if (copy_from_user(&cdata->data[cdata->index++], &buf[i],1))
			return -EFAULT;
	}	
	/* mutex_unlock */
	up(&cdata_sem);
	return 0;
}

static int cdata_release(struct inode *inode, struct file *filp)
{
	printk(KERN_ALERT "cdata: in cdata_release()\n");
	return 0;
}

struct file_operations cdata_fops = {	
	owner:		THIS_MODULE,
	open:		cdata_open,
	release:	cdata_release,
	ioctl:		cdata_ioctl,
	read:		cdata_read,
	write:		cdata_write,
};

int my_init_module(void)
{


	if (register_chrdev(CDATA_MAJOR, "cdata", &cdata_fops)) {

		printk(KERN_ALERT "cdata module: Can not registered.\n");
	}
	else {
		printk(KERN_ALERT "cdata module: registered.\n");
	}

	//cdata->iomem  = ioremap(0x33f00000, LCD_LENGTH);

	return 0;
}

void my_cleanup_module(void)
{
	unregister_chrdev(CDATA_MAJOR, "cdata");
	printk(KERN_ALERT "cdata module: unregisterd.\n");
}

module_init(my_init_module);
module_exit(my_cleanup_module);

MODULE_LICENSE("GPL");

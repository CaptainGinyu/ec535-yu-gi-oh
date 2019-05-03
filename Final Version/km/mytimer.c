/* Necessary includes for device drivers */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/timer.h>

#include <linux/string.h>
#include <linux/ctype.h>

#include <linux/sched.h>
#include <asm/cnt32_to_63.h>
#include <asm/div64.h>

#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/pgtable.h>
#include <asm/mach/map.h>

#include <asm/arch/pxa-regs.h>
#include <asm/arch/gpio.h>
#include <asm/arch/udc.h>
#include <asm/arch/pxafb.h>
#include <asm/arch/mmc.h>
#include <asm/arch/irda.h>
#include <asm/arch/i2c.h>

MODULE_LICENSE("Dual BSD/GPL");

static int fasync_fasync(int fd, struct file *filp, int mode);
static int fasync_open(struct inode *inode, struct file *filp);
static int fasync_release(struct inode *inode, struct file *filp);

static ssize_t fasync_read(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static int fasync_fasync(int fd, struct file *filp, int mode);
static int fasync_init(void);
static void fasync_exit(void);
static void timer_handler();

/*
 * The file operations for the pipe device
 * (some are overlayed with bare scull)
 */
struct file_operations fasync_fops = {
	read: fasync_read,
	open: fasync_open,
	release: fasync_release,
	fasync: fasync_fasync
};

/* Declaration of the init and exit functions */
module_init(fasync_init);
module_exit(fasync_exit);
static int button = 0;
static int fasync_major = 61;
struct fasync_struct *async_queue; /* asynchronous readers */

//set interrupt function for button 0
irqreturn_t gpio_irq0(int irq, void *dev_id, struct pt_regs *regs)
{
	button=0;
	timer_handler();
	printk("wake up\n");
	return IRQ_HANDLED;
}

//set interrupt function for button 1
irqreturn_t gpio_irq1(int irq, void *dev_id, struct pt_regs *regs)
{
	button=1;
	timer_handler();
	printk("wake up\n");
	return IRQ_HANDLED;
}

static int fasync_init(void) {
	int result;
	int irq0 = IRQ_GPIO(29);  //set GPIO 29 as button0
	int irq1 = IRQ_GPIO(30);  //set GPIO 30 as button1
	gpio_direction_input(29);
	gpio_direction_input(30);
	/* Registering device */
	result = register_chrdev(fasync_major, "fasync", &fasync_fops);
	if (result < 0)
	{
		printk(KERN_ALERT
			"fasync: cannot obtain major number %d\n", fasync_major);
		return result;
	}

	
	if (request_irq(irq0, &gpio_irq0, SA_INTERRUPT | SA_TRIGGER_RISING,
				"mygpio", NULL) != 0 ) {
                printk ( "irq not acquired \n" );
                return -1;
        }else{
                printk ( "irq %d acquired successfully \n", irq0 );
	}

	if (request_irq(irq1, &gpio_irq1, SA_INTERRUPT | SA_TRIGGER_RISING,
				"mygpio", NULL) != 0 ) {
                printk ( "irq not acquired \n" );
                return -1;
        }else{
                printk ( "irq %d acquired successfully \n", irq1 );
	}
	printk("mytimer loaded.\n");
	return 0;

fail: 
	fasync_exit(); 
	return result;
}

static void fasync_exit(void) {
	/* Freeing the major number */
	unregister_chrdev(fasync_major, "fasync");
	free_irq(IRQ_GPIO(29), NULL);
	free_irq(IRQ_GPIO(30), NULL);

	printk(KERN_ALERT "Removing mytimer module\n");

}

static int fasync_open(struct inode *inode, struct file *filp) {
	return 0;
}

static int fasync_release(struct inode *inode, struct file *filp) {
	fasync_fasync(-1, filp, 0);
	return 0;
}


static ssize_t fasync_read(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
	char output[10];
	sprintf(output,"");
	sprintf(output, "%d", button);
	count=strlen(output);
	if(copy_to_user(buf, output, strlen(output)))
	  {return -EFAULT;}
	//printk("FROM KERNEL: %s", output);
	button = 0;
	count = 0;
	return count;
}

static int fasync_fasync(int fd, struct file *filp, int mode) {
	return fasync_helper(fd, filp, mode, &async_queue);
}

static void timer_handler() {
	if (async_queue){
		kill_fasync(&async_queue, SIGIO, POLL_IN);
	}
}


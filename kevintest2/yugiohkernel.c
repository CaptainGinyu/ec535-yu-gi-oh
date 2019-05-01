#include <linux/module.h>
#include <linux/interrupt.h>
#include <asm/arch/pxa-regs.h>
#include <asm-arm/arch/hardware.h>

#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */

#include <asm/arch/pxa-regs.h>
#include <asm/arch/gpio.h>

const int BUTTON0 = 28;
const int BUTTON1 = 31;
const int BUTTON2 = 29;
const int BUTTON3 = 30;
const int major = 61;

static int button_open(struct inode *inode, struct file *filp);
static int button_release(struct inode *inode, struct file *filp);
static ssize_t button_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
static void my_cleanup_module(void);
static int my_init_module(void);

struct file_operations button_fops = {
	read: button_read,
	open: button_open,
	release: button_release
};

static int button0_val;
static int button1_val;
static int button2_val;
static int button3_val;

static char previousOutput[] = "";

irqreturn_t gpio_irq0(int irq, void *dev_id, struct pt_regs *regs)
{
	//no debounce, active for both edges
	//printk("Button IRQ 0\n");

    button0_val = pxa_gpio_get_value(BUTTON0);

	return IRQ_HANDLED;
}

irqreturn_t gpio_irq1(int irq, void *dev_id, struct pt_regs *regs)
{
	//no debounce, active for both edges
	//printk("Button IRQ 1\n");

    button1_val = pxa_gpio_get_value(BUTTON1);

	return IRQ_HANDLED;
}

irqreturn_t gpio_irq2(int irq, void *dev_id, struct pt_regs *regs)
{
	//no debounce, active for both edges
	//printk("Button IRQ 2\n");

    button2_val = pxa_gpio_get_value(BUTTON2);

	return IRQ_HANDLED;
}

irqreturn_t gpio_irq3(int irq, void *dev_id, struct pt_regs *regs)
{
	//no debounce, active for both edges
	//printk("Button IRQ 3\n");

    button3_val = pxa_gpio_get_value(BUTTON3);

	return IRQ_HANDLED;
}

static int my_init_module(void)
{
	//printk("Inserting Yu-Gi-Oh kernel module\n");

    int result;

    /* Registering device */
	result = register_chrdev(major, "yugiohkernel", &button_fops);
	if (result < 0)
	{
		//printk(KERN_ALERT "yugiohkernel: cannot obtain major number %d\n", major);
		return result;
	}

	pxa_gpio_mode(BUTTON0 | GPIO_IN);
	int irq0 = IRQ_GPIO(BUTTON0);
	if (request_irq(irq0, &gpio_irq0, SA_INTERRUPT | SA_TRIGGER_RISING| SA_TRIGGER_FALLING,
				"yugiohkernel", NULL) != 0 )
    {
        //printk ( "irq0 not acquired \n" );
        return -1;
    }
    else
    {
        //printk ( "irq %d acquired successfully \n", irq0 );
	}

    pxa_gpio_mode(BUTTON1 | GPIO_IN);
	int irq1 = IRQ_GPIO(BUTTON1);
	if (request_irq(irq1, &gpio_irq1, SA_INTERRUPT | SA_TRIGGER_RISING | SA_TRIGGER_FALLING,
				"yugiohkernel", NULL) != 0 )
    {
        //printk ( "irq1 not acquired \n" );
        return -1;
    }
    else
    {
        //printk ( "irq %d acquired successfully \n", irq1 );
	}

    pxa_gpio_mode(BUTTON2 | GPIO_IN);
	int irq2 = IRQ_GPIO(BUTTON2);
	if (request_irq(irq2, &gpio_irq2, SA_INTERRUPT | SA_TRIGGER_RISING | SA_TRIGGER_FALLING,
				"yugiohkernel", NULL) != 0 )
    {
        //printk ( "irq2 not acquired \n" );
        return -1;
    }
    else
    {
        //printk ( "irq %d acquired successfully \n", irq2 );
	}

    pxa_gpio_mode(BUTTON3 | GPIO_IN);
	int irq3 = IRQ_GPIO(BUTTON3);
	if (request_irq(irq3, &gpio_irq3, SA_INTERRUPT | SA_TRIGGER_RISING | SA_TRIGGER_FALLING,
				"yugiohkernel", NULL) != 0 )
    {
        //printk ( "irq3 not acquired \n" );
        return -1;
    }
    else
    {
        //printk ( "irq %d acquired successfully \n", irq3 );
	}
    
	return 0;
}

static ssize_t button_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
    char output[] = "0000";

    count = 4;

    if (button0_val)
    {
        output[0] = '1';
    }
    if (button1_val)
    {
        output[1] = '1';
    }
    if (button2_val)
    {
        output[2] = '1';
    }
    if (button3_val)
    {
        output[3] = '1';
    }

    if (strcmp(output, previousOutput) == 0)
    {
        return 0;
    }
    strcpy(previousOutput, output);

    /*if (*f_pos >= count)
	{
		return 0;
	}*/

    if(copy_to_user(buf, output, count))
	{
		return -EFAULT;
	}

    //*f_pos += count;
    return count;
}

static int button_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int button_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static void my_cleanup_module(void)
{
	free_irq(IRQ_GPIO(BUTTON0), NULL);
    free_irq(IRQ_GPIO(BUTTON1), NULL);
    free_irq(IRQ_GPIO(BUTTON2), NULL);
    free_irq(IRQ_GPIO(BUTTON3), NULL);

    unregister_chrdev(major, "yugiohkernel");

	//printk("Removing Yu-Gi-Oh kernel module!\n");
}


module_init(my_init_module);
module_exit(my_cleanup_module);

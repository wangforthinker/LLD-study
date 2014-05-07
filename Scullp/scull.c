#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include "scull.h"

extern int scull_major;
extern int scull_minor;

struct file_operations scull_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.read = scullp_read,
	.write = scullp_write,
	.open = scullp_open,
	.release = scullp_release,
	.unlocked_ioctl = scullp_ioctl,
};

int scullp_ioctl(struct file* filp , unsigned int cmd , unsigned long arg)
{
	int err = 0;
	int retval = 0;
	
	printk(KERN_ALERT"In scull_ioctl , arg is %ld\n",arg);
	if(_IOC_TYPE(cmd) != SCULL_IOC_MAGIC)
	{
		printk("In scull_ioctl ,IOC_TYPE error\n");
		return -ENOTTY;
	}

	if(_IOC_NR(cmd) > SCULL_IOC_MAXNR)
	{	
		printk("In scull_ioctl ,IOC_NR error\n");
		return -ENOTTY;
	}

	if(_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE , (void __user*)arg , _IOC_SIZE(cmd));
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERUFY_READ , (void __user*)arg , _IOC_SIZE(cmd));
	if(err)
	{
		printk("In scull_ioctl ,IOC_DIR error\n");
		return -EFAULT;
	}

	struct scullp_dev* dev = filp->private_data;
	
	if(down_interruptible(&dev->sem))
	{
		printk("down_interruptible error\n");
		return -ERESTARTSYS;
	}
		
/*	switch(cmd)
	{
		case SCULL_IOCRESET:
			dev->quantum = SCULL_QUANTUM;
			dev->qset = SCULL_QSET;
			break;
		case SCULL_IOCSQUANTUM:
			dev->quantum = (long __user*)arg;
			printk("In SCULL_IOCSQUANTUM , dev->quantum set is %ld\n",(long __user*)arg);
			break;
		case SCULL_IOCTQUANTUM:
			dev->quantum = arg;
			printk("In SCULL_IOCTQUANTUM , dev->quantum set is %ld\n",arg);
			break;
		case SCULL_IOCGQUANTUM:
			printk("In SCULL_IOCGQUANTUM\n");
			__put_user(dev->quantum ,(long __user*)arg);
			break;
		case SCULL_IOCQQUANTUM:
			retval = dev->quantum;
			printk("In SCULL_IOCQQUANTUM retval is %d\n",retval);
			break;
		default:
			break;
	}
*/	
	up(&dev->sem);
	return retval;
	
}


loff_t scullp_seek(struct file* filp , loff_t index , int counts )
{
	struct scullp_dev *dev = filp->private_data;
	loff_t newops;
	
	switch(counts)
	{
		case 0:
			newops = index;
			break;
		case 1:
			newops = filp->f_pos + index;
			break;
		case 2:
			newops = dev->bufferSize + index;
			break;
		default:
			return -EINVAL;
	} 
	
	if(newops < 0)
		return -EINVAL;
	
	printk(KERN_ALERT"in scull seek\n");
	filp->f_pos = newops;
	return newops;
}

ssize_t scullp_read(struct file* filp , char __user* data , size_t count , loff_t* f_pos)
{
	struct scullp_dev* dev = filp->private_data;
	int retval = 0;
	
	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	
	while(dev->wp == dev->rp)
	{
		up(&dev->sem);
		if(filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		
		if(wait_event_interruptible(dev->inq, dev->wp != dev->rp))
			return -ERESTARTSYS;
		
		if(down_interruptible(&dev->sem))
			return -ERESTARTSYS;
	}
	

	if(dev->wp > dev->rp)
		count = min(count,(size_t)(dev->wp - dev->rp));
	else
		count = min(count,(size_t)(dev->end - dev->rp));
	
	printk("In scullp_read , count is %d\n",count);
	if(copy_to_user(data , dev->rp , count))
	{
		up(&dev->sem);
		return -EFAULT;
	}

	dev->rp += count;
	if(dev->rp == dev->end)
		dev->rp = dev->buffer;
		
	up(&dev->sem);

	wake_up_interruptible(&dev->outq);
	printk("In Scullp_read , wake up writer queue, i am %d\n",current->pid);
	return count;
	
}

int getFreeSpace(struct scullp_dev* dev)
{
	if(dev->rp == dev->wp)
		return (dev->bufferSize - 1);

	return ((dev->rp + dev->bufferSize - dev->wp)%(dev->bufferSize) - 1);
	
}

ssize_t scullp_write(struct file* filp , const char __user* data , size_t count , loff_t* f_pos)
{
	struct scullp_dev* dev = filp->private_data;
	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;
		
	while(getFreeSpace(dev) == 0)
	{
		up(&dev->sem);
		if(filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		
		if(wait_event_interruptible(dev->outq , getFreeSpace(dev) != 0))
			return -EAGAIN;
		
		if(down_interruptible(&dev->sem))
			return -ERESTARTSYS;
	}
	
	printk("In scullp_write , count is %d\n",count);
	
	if(dev->rp > dev->wp)
		count = min(count , (size_t)(dev->rp - dev->wp - 1));
	else
		count = min(count , (size_t)(dev->end - dev->wp));
	
	printk("In scullp_write, count is %d\n",count);
	if(copy_from_user(dev->wp , data , count))
	{
		printk("In scullp_write , copy_from_user error\n");
		up(&dev->sem);
		return -EFAULT;
	}
	
	dev->wp += count;
	if(dev->wp == dev->end)
		dev->wp = dev->buffer;

	
	up(&dev->sem);
	wake_up_interruptible(&dev->inq);
	printk("In scullp_write , wake up reader ,i am %d\n",current->pid);
		
	return count;
}

int scullp_open(struct inode* inode , struct file* filp)
{
	struct scullp_dev* dev;
	dev = container_of(inode->i_cdev , struct scullp_dev , cdev);
	filp->private_data = dev;
	nonseekable_open(inode , filp);
		
	if((filp->f_flags & O_ACCMODE) == O_WRONLY)
	{
		dev->wp = dev->rp = dev->buffer;

	}
	return 0;
}

int scullp_release(struct inode* inode , struct file* filp)
{
//	struct scull_dev* dev = filp->private_data;
//	scull_trim(dev);
	printk("in scull_release\n");
	return 0;
}

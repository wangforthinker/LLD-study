#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include "scull.h"

extern int scull_major;
extern int scull_minor;

struct file_operations scull_fops = {
	.owner = THIS_MODULE,
	.llseek = scull_seek,
	.read = scull_read,
	.write = scull_write,
	.open = scull_open,
	.release = scull_release,
	.unlocked_ioctl = scull_ioctl,
};

int scull_ioctl(struct file* filp , unsigned int cmd , unsigned long arg)
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

	struct scull_dev* dev = filp->private_data;
	
	if(down_interruptible(&dev->sem))
	{
		printk("down_interruptible error\n");
		return -ERESTARTSYS;
	}
		
	switch(cmd)
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
	
	up(&dev->sem);
	return retval;
	
}

struct scull_qset* scull_follow(struct scull_dev* dev , int item)
{	
	struct scull_qset* first = dev->data;

	if(!first)
	{
		first = kmalloc(sizeof(struct scull_qset) , GFP_KERNEL);
		if(!first)
			return NULL;
		
		first->data = NULL;
		first->next = NULL;
		dev->data = first;	
	}
	
	int i = 0;
	struct scull_qset* dptr = first;
	struct scull_qset* ptr = NULL;
		
	while(true)
	{
		if(!dptr)
		{
			printk("In dptr\n");
			dptr = kmalloc(sizeof(struct scull_qset) , GFP_KERNEL);
			if(!dptr)
				return NULL;
			
			printk("dptr is %ld\n",dptr);
			dptr->data = NULL;
			dptr->next = NULL;
			ptr->next = dptr;
		}

		if(item == i)
			break;

		ptr = dptr;
		dptr = dptr->next;
		i++;
	}

	return dptr;
}

int scull_trim(struct scull_dev* dev)
{
	struct scull_qset* next, *dptr;
	int qset = dev->qset;
	int i ;

	printk("In scull_trim \n");
	for(dptr = dev->data ; dptr ; dptr = next)
	{
		if(dptr->data)
		{	
			printk("dptr->data is %ld\n",dptr->data);
			for(i = 0; i< qset ; i++)
			{
				if(dptr->data[i])
				{	
					printk("dptr->data[%d] is %ld\n",i,dptr->data[i]);
					kfree(dptr->data[i]);
					dptr->data[i] = NULL;
				}
			}
			kfree(dptr->data);
			dptr->data = NULL;
		}
		next = dptr->next;
		kfree(dptr);
	}
	
	dev->size = 0;
	dev->quantum = SCULL_QUANTUM;
	dev->qset = SCULL_QSET;
	dev->data = NULL;
	return 0;
}

loff_t scull_seek(struct file* filp , loff_t index , int counts )
{
	struct scull_dev *dev = filp->private_data;
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
			newops = dev->size + index;
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

ssize_t scull_read(struct file* filp , char __user* data , size_t count , loff_t* f_pos)
{
	struct scull_dev* dev = filp->private_data;
	struct scull_qset* dptr;
	int quantum = dev->quantum;
	int qset = dev->qset;
	int item , s_pos , q_pos , rset;
	int itemsize = quantum * qset;
	
	int retval = 0;
	
	if(dev->size == 0)
	{	
		printk("In scull_read wait\n");
		wait_for_completion(&dev->com);
	}

	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	
	printk(KERN_ALERT "dev size is %d, f_pos is %ld\n", dev->size, *f_pos);
	if(*f_pos >= dev->size)
		goto out;
	
	printk(KERN_ALERT"f_pos ok\n");
	if(*f_pos + count > dev->size)
		count = dev->size - *f_pos;
	
	item = (int)*f_pos / itemsize;
	rset = (int)*f_pos % itemsize;
	
	s_pos = rset / quantum;
	q_pos = rset % quantum;
	
	printk("item is %d , rset is %d , s_pos is %d , q_pos is %d , count is %d,dev->data address is %ld\n",item , rset , s_pos , q_pos ,count , dev->data);
	dptr = scull_follow(dev , item);
	
	if(dptr == NULL )
	{
		printk(KERN_ALERT"dptr NULL\n");
		goto out;
	}
	
	 printk("In scull_read dptr address is %ld , dptr->data address is %ld\n",dptr , dptr->data);

	if(dptr->data == NULL)
	{
		printk("dptr->data NULL\n");
		goto out;
	}
	
	if(dptr->data[s_pos] == NULL)
	{
		printk("dptr->data[%d] NULL\n",s_pos);
		goto out;	
	}
	
	if(count > quantum - q_pos)
		count = quantum - q_pos;

	printk(KERN_ALERT"count is %d\n",count);
	unsigned long OutSize = copy_to_user(data , dptr->data[s_pos] + q_pos , count);
	if(OutSize)
	{
		retval = -EFAULT;
		printk(KERN_ALERT"copy_to_user error,ret is %ld\n",OutSize);
		goto out;
	}
	
	printk(KERN_ALERT"read from scull\n");
	*f_pos += count;
	retval = count;

out:
	up(&dev->sem);
	return retval;
	
}

ssize_t scull_write(struct file* filp , const char __user* data , size_t count , loff_t* f_pos)
{
	struct scull_dev* dev = filp->private_data;
	struct scull_qset* dptr;
	int quantum = dev->quantum;
	int qset = dev->qset;
	int item , s_pos , q_pos , rset;
	int itemsize = quantum * qset;
	
	printk(KERN_ALERT"scull in write\n");
	int retval = ENOMEM;
	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	
	item = (int)*f_pos / itemsize;
	rset = (int)*f_pos % itemsize;
	s_pos = rset / quantum;
	q_pos = rset % quantum;
	
	printk("itemsize is %d ,item is %d , rset is %d , s_pos is %d , q_pos is %d ,dev->data address is %ld\n",itemsize , item , rset , s_pos , q_pos);
	dptr = scull_follow(dev , item);
	
	if(!dptr)
		goto out;
	
	if(!dptr->data)
	{
		printk("dptr->data is %ld\n",dptr->data);
		dptr->data = kmalloc(qset * sizeof(char*) , GFP_KERNEL);
		if(!dptr->data)
			goto out;
		int j = 0;	
		for(;j < qset ; j++)
			dptr->data[j] = NULL;
	}
	
	printk("In scull_write dptr address is %ld , dptr->data address is %ld\n",dptr , dptr->data);

	if(!dptr->data[s_pos])
	{
		dptr->data[s_pos] = kmalloc(quantum , GFP_KERNEL);
		if(!dptr->data[s_pos])
			goto out;
		
		printk("dptr->data[%d] is %ld\n",s_pos , dptr->data[s_pos]);
	//	memset(dptr->data[s_pos] , 0 , quantum);
	}

	if(count > quantum - q_pos)
		count = quantum - q_pos;

	if(copy_from_user(dptr->data[s_pos] + q_pos , data , count))
	{
		retval = -EFAULT;
		goto out;
	}
	
	printk(KERN_ALERT"write in scull\n");
	*f_pos += count;
	retval = count;
	if(*f_pos > dev->size)
	{
		printk(KERN_ALERT "dev size is %d, f_pos is %ld\n", dev->size, *f_pos);
		dev->size = *f_pos;
		printk(KERN_ALERT "dev size is %d, f_pos is %ld\n", dev->size, *f_pos);
	}
	
	complete(&dev->com);
out:
	up(&dev->sem);
	return retval;
}

int scull_open(struct inode* inode , struct file* filp)
{
	struct scull_dev* dev;
	dev = container_of(inode->i_cdev , struct scull_dev , cdev);
	filp->private_data = dev;
	
	if((filp->f_flags & O_ACCMODE) == O_WRONLY)
	{
		scull_trim(dev);
	}
	return 0;
}

int scull_release(struct inode* inode , struct file* filp)
{
//	struct scull_dev* dev = filp->private_data;
//	scull_trim(dev);
	printk("in scull_release\n");
	return 0;
}

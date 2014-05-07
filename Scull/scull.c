#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include "scull.h"

MODULE_LICENSE("Dual BSD/GPL");

static int scull_major = 0;
static int scull_minor = 0;
static unsigned int scull_nr_devs = 4;

extern struct file_operations scull_fops;
struct scull_dev dev_info[4]; 

static int getDevNum(void)
{
	dev_t dev;
	int r = 0;
	if(scull_major)
	{
		dev = MKDEV(scull_major, scull_minor );
		r = register_chrdev_region(dev, scull_nr_devs, "scull");
		
	}
	else
	{
		r = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs ,"scull");
		scull_major = MAJOR(dev);
	}

	if(r < 0)
	{
		printk(KERN_WARNING"scull: cannot get major %d\n", scull_major);
		return r;
	}
	
	printk(KERN_ALERT"scull major is %d\n", scull_major);
	return r;
}

static void scull_setup_cdev(struct scull_dev* dev , int index )
{
        int err;
        dev_t devno = MKDEV(scull_major , scull_minor + index );
        cdev_init(&dev->cdev , &scull_fops);
        dev->cdev.owner = THIS_MODULE;
        dev->cdev.ops = &scull_fops;
        err = cdev_add(&dev->cdev , devno , 1);

        if(err)
                printk(KERN_NOTICE"Error %d adding scull%d\n", err , index);
}

static void scull_close_cdev(struct scull_dev* dev)
{
        cdev_del(&dev->cdev);
}

static int scull_init(void)
{
	int r = getDevNum();
	if(r < 0)
		return r;

	int i;
	for(i = 0; i < scull_nr_devs ; i++)
	{
		dev_info[i].quantum = SCULL_QUANTUM;
		dev_info[i].qset = SCULL_QSET;
		dev_info[i].size = 0;
		dev_info[i].data = NULL;
		sema_init(&dev_info[i].sem,1);
		init_completion(&dev_info[i].com);
		scull_setup_cdev(&dev_info[i] , i );
	}
	return 0;
}

static void scull_exit(void)
{
//	scull_trim(&dev_info);
	
	int i ;
	for(i = 0; i < scull_nr_devs ; i++) 
	{	
		scull_trim(&dev_info[i]);	
		scull_close_cdev(&dev_info[i]);
	}
	dev_t dev = MKDEV(scull_major , scull_minor);
	unregister_chrdev_region(dev , scull_nr_devs);
}

module_init(scull_init);
module_exit(scull_exit);




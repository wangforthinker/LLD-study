#ifndef SCULL_H
#define SCULL_H

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#include <linux/ioctl.h>

#define SCULL_QUANTUM 4000
#define SCULL_QSET 1000
#define SCULL_IOC_MAGIC 'k'

#define SCULL_IOCRESET _IO(SCULL_IOC_MAGIC , 0)

#define SCULL_IOCSQUANTUM _IOW(SCULL_IOC_MAGIC , 1 , long)
#define SCULL_IOCTQUANTUM _IO(SCULL_IOC_MAGIC , 2)
#define SCULL_IOCGQUANTUM _IOR(SCULL_IOC_MAGIC , 3 , long)
#define SCULL_IOCQQUANTUM _IO(SCULL_IOC_MAGIC , 4)


#define SCULL_IOC_MAXNR 5
#define BUFFER_SIZE 4096

struct scullp_dev
{
	wait_queue_head_t inq , outq;
	char* buffer,*end;
	int bufferSize;
	char* rp , *wp;
	struct semaphore sem;
	struct cdev cdev;
};

//void scull_setup_cdev(struct scull_dev* dev , int index);
int scullp_ioctl(struct file* , unsigned int , unsigned long);
//int scullp_trim(struct scull_dev* dev);
loff_t scullp_seek(struct file* , loff_t , int );
ssize_t scullp_read(struct file* , char __user* , size_t ,loff_t* );
ssize_t scullp_write(struct file* , const char __user* , size_t , loff_t* );
int scullp_open(struct inode* , struct file* );
int scullp_release(struct inode* , struct file* );

#endif

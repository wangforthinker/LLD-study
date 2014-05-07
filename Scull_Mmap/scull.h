#ifndef SCULL_H
#define SCULL_H

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/semaphore.h>
#include <linux/completion.h>
#include <linux/ioctl.h>
#include <linux/mm.h>

#define SCULL_QUANTUM 4096
#define SCULL_QSET 1000
#define SCULL_IOC_MAGIC 'k'

#define SCULL_IOCRESET _IO(SCULL_IOC_MAGIC , 0)

#define SCULL_IOCSQUANTUM _IOW(SCULL_IOC_MAGIC , 1 , long)
#define SCULL_IOCTQUANTUM _IO(SCULL_IOC_MAGIC , 2)
#define SCULL_IOCGQUANTUM _IOR(SCULL_IOC_MAGIC , 3 , long)
#define SCULL_IOCQQUANTUM _IO(SCULL_IOC_MAGIC , 4)


#define SCULL_IOC_MAXNR 5

struct scull_qset
{
	void** data;
	struct scull_qset* next;
};

struct scull_dev
{
	struct scull_qset* data;
	int mmap_count;	
	int quantum;
	int qset;
	unsigned long size;
	struct semaphore sem;
	struct completion com;
	struct cdev cdev;
};

//void scull_setup_cdev(struct scull_dev* dev , int index);
int scull_ioctl(struct file* , unsigned int , unsigned long);
int scull_trim(struct scull_dev* dev);
loff_t scull_seek(struct file* , loff_t , int );
ssize_t scull_read(struct file* , char __user* , size_t ,loff_t* );
ssize_t scull_write(struct file* , const char __user* , size_t , loff_t* );
int scull_open(struct inode* , struct file* );
int scull_release(struct inode* , struct file* );

int scull_mmap(struct file* filp , struct vm_area_struct* vma);
void scull_vma_open(struct vm_area_struct* vma);
void scull_vma_close(struct vm_area_struct* vma);
int scull_vma_fault(struct vm_area_struct* vma , struct vm_fault* vm_ft);
#endif

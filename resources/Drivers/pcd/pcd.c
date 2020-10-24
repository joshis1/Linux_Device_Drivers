#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

dev_t device_number;
struct cdev pcd_cdev;

struct class *class_pcd;
struct device *device_pcd;

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence);
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
int pcd_open(struct inode *inode, struct file *filp);
int pcd_release(struct inode *inode, struct file *flip);


/**Trick to get the function name **/
#undef pr_fmt
#define pr_fmt(fmt) "%s : "fmt,__func__

struct file_operations pcd_fops =
{
  .open = pcd_open,
  .release = pcd_release,
  .read = pcd_read,
  .write = pcd_write,
  .llseek = pcd_lseek,
  .owner = THIS_MODULE

};

/**Entry Point of the Kernel Module **/
/** Called when the module is inserted -insmod **/
static int __init pcd_driver_init(void)
{
   int ret;
   /* 1. Dynamically allocate a device number **/
   ret = alloc_chrdev_region(&device_number,0,1, "pcd_devices");
   if( ret < 0){
	   pr_err("Alloc chrdev failed\n");
   }

   pr_info("Device number <major>:<minor> = %d:%d\n", MAJOR(device_number), MINOR(device_number));

   //Initialize the cdev structure with file operations 
   cdev_init(&pcd_cdev, &pcd_fops);

   //Register a device ( cdev stucture) with VFS 
   pcd_cdev.owner = THIS_MODULE;
   ret= cdev_add(&pcd_cdev, device_number, 1);
   if(ret < 0){
          pr_err("Cdev add failed\n");
   }
   // create device class under /sys/class
   class_pcd = class_create(THIS_MODULE, "pcd_class");
  
   // populate the sysfs with device information
   device_pcd = device_create(class_pcd, NULL, device_number, NULL, "pcd");

   pr_info("Module init was successful\n");

   return ret;
}

/** Called when the module is removed - rmmod**/
static void __exit pcd_driver_cleanup(void)
{
   device_destroy(class_pcd, device_number);
   class_destroy(class_pcd);
   cdev_del(&pcd_cdev);
   unregister_chrdev_region(device_number, 1);
   pr_info("module unloaded\n");
}

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{
  pr_info("lseek was invoked count = %zu\n",offset);
  return 0;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
  pr_info("read was invoked count = %zu\n",count);
  return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
  pr_info("write was invoked count = %zu\n",count);
  return 0;
}

int pcd_open(struct inode *inode, struct file *filp)
{
  pr_info("open was invoked\n");
  return 0;
}

int pcd_release(struct inode *inode, struct file *flip)
{
  pr_info("release was invoked\n");
  return 0;
}

/**module registration **/
module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL"); //GPL - check module.h - very important 
MODULE_AUTHOR("Shreyas Joshi");
MODULE_DESCRIPTION("Hello World Program");
MODULE_INFO(board, "Zedboard - AVNET");




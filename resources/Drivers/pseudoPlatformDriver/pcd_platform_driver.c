#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
/** for copy_to_user and copy_from_user **/
#include <linux/uaccess.h> 
/** For IS_ERR(), PTR_ERR(), and ERR_PTR() **/
#include <linux/err.h>
/** container_of **/
#include<linux/kernel.h>
#include "platform.h"
#include <linux/platform_device.h>

/**private static prototypes**/
//static int check_permission(fmode_t mode, int perm);


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

int pcd_platform_driver_probe (struct platform_device *pDev)
{
    pr_info("pcd_platform_driver_probe\n");
    return 0;
}

int pcd_platform_driver_remove(struct platform_device *pDev)
{
   pr_info("pcd_platform_driver_remove\n");
   return 0;
}

struct platform_driver pcd_platform_driver = {
	.probe =  pcd_platform_driver_probe,
	.remove = pcd_platform_driver_remove,
	.driver = {
          .name =  "pseudo-char-device",
	},	
};

/**Entry Point of the Kernel Module **/
/** Called when the module is inserted -insmod **/
static int __init pcd_platform_driver_init(void)
{
   pr_info("pcd platform driver module insert\n");
   platform_driver_register(&pcd_platform_driver); 	
   return 0;  
}

/** Called when the module is removed - rmmod**/
static void __exit pcd_platform_driver_cleanup(void)
{
   pr_info("module unloaded\n");
   platform_driver_unregister(&pcd_platform_driver); 
}

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence)
{
  return 0;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
   return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
  return 0;
}

int pcd_open(struct inode *inode, struct file *filp)
{
  return 0;
}

int pcd_release(struct inode *inode, struct file *flip)
{
  pr_info("release was invoked\n");
  return 0;
}

#if 0
static int check_permission(fmode_t mode, int perm)
{
   if(perm == RDWR) {
      pr_info("Permission granted - RW\n");
      return 0;
   }
   else if((perm == RONLY) && (mode & FMODE_READ) && !(mode & FMODE_WRITE)) {
       pr_info("Permission granted - RONLY\n");
       return 0;
   }
   else if((perm == WRONLY) && (mode & FMODE_WRITE) && !(mode & FMODE_READ)) {
       pr_info("Permission granted - WRONLY\n");
       return 0;
   }
   else {
        return -EPERM;
    }
}
#endif
/**module registration **/
module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_cleanup);

/*GPL - check module.h - very important */
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Shreyas Joshi");
MODULE_DESCRIPTION("Pseudo platform device driver");
MODULE_INFO(board, "Zedboard - AVNET");


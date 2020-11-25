#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
/** for copy_to_user and copy_from_user **/
#include <linux/uaccess.h>
/** For IS_ERR(), PTR_ERR(), and ERR_PTR() **/
#include <linux/err.h>
#include <linux/mutex.h>

dev_t device_number;
struct cdev pcd_cdev;

struct class *class_pcd;
struct device *device_pcd;

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence);
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count,
                 loff_t *f_pos);
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count,
                  loff_t *f_pos);
int pcd_open(struct inode *inode, struct file *filp);
int pcd_release(struct inode *inode, struct file *flip);

#define DEV_SIZE (256)
char device_memory[DEV_SIZE] = "Hello World \n";

DEFINE_MUTEX(pcd_mutex_lock);

/**Trick to get the function name **/
#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt, __func__

struct file_operations pcd_fops = {.open = pcd_open,
                                   .release = pcd_release,
                                   .read = pcd_read,
                                   .write = pcd_write,
                                   .llseek = pcd_lseek,
                                   .owner = THIS_MODULE

};

/**Entry Point of the Kernel Module **/
/** Called when the module is inserted -insmod **/
static int __init pcd_driver_init(void) {
  int ret;
  /** 1. Dynamically allocate a device number **/
  ret = alloc_chrdev_region(&device_number, 0, 1, "pcd_devices");
  if (ret < 0) {
    pr_err("Alloc chrdev failed\n");
    goto out;
  }

  pr_info("Device number <major>:<minor> = %d:%d\n", MAJOR(device_number),
          MINOR(device_number));

  /*Initialize the cdev structure with file operations*/
  cdev_init(&pcd_cdev, &pcd_fops);

  /*Register a device ( cdev stucture) with VFS */
  pcd_cdev.owner = THIS_MODULE;
  ret = cdev_add(&pcd_cdev, device_number, 1);
  if (ret < 0) {
    pr_err("Cdev add failed\n");
    goto unreg_chrdev;
  }
  /*create device class under /sys/class */
  class_pcd = class_create(THIS_MODULE, "pcd_class");
  if (IS_ERR(class_pcd)) {
    pr_err("Error creating class\n");
    ret = PTR_ERR(class_pcd);
    goto cdev_del;
  }

  /*populate the sysfs with device information */
  device_pcd = device_create(class_pcd, NULL, device_number, NULL, "pcd");
  /** IS_ERR(), PTR_ERR() and ERR_PTR() defined in /linux/err.h **/
  /** PTR_ERR - > converts pointer to error number value **/
  /** ERR_PTR() -> converts the error number to the pointer**/
  if (IS_ERR(device_pcd)) {
    pr_err("Device Create failed\n");
    ret = PTR_ERR(device_pcd);
    goto class_del;
  }

  pr_info("Module init was successful\n");

  return 0;

class_del:
  class_destroy(class_pcd);
cdev_del:
  cdev_del(&pcd_cdev);
unreg_chrdev:
  unregister_chrdev_region(device_number, 1);
out:
  pr_err("Module init failed\n");
  return ret;
}

/** Called when the module is removed - rmmod**/
static void __exit pcd_driver_cleanup(void) {
  device_destroy(class_pcd, device_number);
  class_destroy(class_pcd);
  cdev_del(&pcd_cdev);
  unregister_chrdev_region(device_number, 1);
  pr_info("module unloaded\n");
}

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence) {
  loff_t temp;
  pr_info("lseek was invoked count = %llu and fpos = %lld\n", offset,
          filp->f_pos);

  switch (whence) {
  case SEEK_SET:
    temp = offset;
    if ((offset > DEV_SIZE) || (offset < 0)) {
      return -EINVAL;
    }
    filp->f_pos = offset;
    break;
  case SEEK_CUR:
    temp = filp->f_pos + offset;
    if ((temp > DEV_SIZE) || temp < 0) {
      return -EINVAL;
    }
    filp->f_pos = temp;
    break;
  case SEEK_END:
    temp = offset + DEV_SIZE;
    if ((temp > DEV_SIZE) || temp < 0) {
      return -EINVAL;
    }
    filp->f_pos = temp;
    break;
  }

  pr_info("New f_pos = %lld\n", filp->f_pos);
  return filp->f_pos;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count,
                 loff_t *f_pos) {
  loff_t temp;
  int ret;
  pr_info("read was invoked count = %zu and f_pos = %lld\n", count, *f_pos);
  pr_info("Fpos = %llu \n", *f_pos);

  ret = mutex_lock_interruptible(&pcd_mutex_lock);
  if (ret)
    return ret;

  temp = count + (*f_pos);

  if (temp > (DEV_SIZE - *(f_pos))) {
    pr_info("Copying the data -- count is more than required\n");
    count = DEV_SIZE - *f_pos;
    ret = copy_to_user(buff, &device_memory[*f_pos], count);
    if (ret < 0) {
      pr_err("Copy to user failed = %d\r\n", ret);
      mutex_unlock(&pcd_mutex_lock);
      return -EFAULT;
    }
    *f_pos = DEV_SIZE;
    pr_info("New f_pos = %llu and bytes written = %zu\n", *f_pos, count);
    mutex_unlock(&pcd_mutex_lock);
    return count;
  }

  else {
    pr_info("Copy the remaining data\n");
    ret = copy_to_user(buff, &device_memory[*f_pos], count);
    if (ret < 0) {
      pr_err("Copy to user failed = %d\r\n", ret);
      mutex_unlock(&pcd_mutex_lock);
      return -EFAULT;
    }
    *f_pos += count;
    pr_info("New f_pos = %llu\n", *f_pos);
    mutex_unlock(&pcd_mutex_lock);
    return count;
  }

  mutex_unlock(&pcd_mutex_lock);

  return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count,
                  loff_t *f_pos) {
  loff_t temp;
  int ret;
  pr_info("write was invoked count = %zu and f_pos = %lld\n", count, *f_pos);

  ret = mutex_lock_interruptible(&pcd_mutex_lock);
  if (ret)
    return ret;

  temp = count + (*f_pos);

  if (temp > (DEV_SIZE - *(f_pos))) {
    pr_info("Writing the data --over flow\n");
    mutex_unlock(&pcd_mutex_lock);
    return -ENOMEM; /*check out of memory error code.*/
  }

  else {
    pr_info("Write the remaining data\n");
    ret = copy_from_user(&device_memory[*f_pos], buff, count);
    if (ret < 0) {
      pr_err("Copy from user failed = %d\r\n", ret);
      mutex_unlock(&pcd_mutex_lock);
      return -EFAULT;
    }

    *f_pos += count;
    pr_info("Updated f_pos = %lld\n", *f_pos);
    mutex_unlock(&pcd_mutex_lock);
    return count;
  }

  pr_info("Updated f_pos = %lld\n", *f_pos);

  mutex_unlock(&pcd_mutex_lock);

  return 0;
}

int pcd_open(struct inode *inode, struct file *filp) {
  pr_info("open was invoked\n");
  return 0;
}

int pcd_release(struct inode *inode, struct file *flip) {
  pr_info("release was invoked\n");
  return 0;
}

/**module registration **/
module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

/*GPL - check module.h - very important */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shreyas Joshi");
MODULE_DESCRIPTION("Pseudo Character Driver");
MODULE_INFO(board, "Zedboard - AVNET");

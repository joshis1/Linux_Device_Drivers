#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
/** for copy_to_user and copy_from_user **/
#include <linux/uaccess.h>
/** For IS_ERR(), PTR_ERR(), and ERR_PTR() **/
#include <linux/err.h>
/** container_of **/
#include <linux/kernel.h>
/** kzalloc **/
#include <linux/slab.h>

#include "platform.h"
#include <linux/platform_device.h>
/**platform_device_id - match based out of platform_device_id **/
#include <linux/mod_devicetable.h>

/**private static prototypes**/
// static int check_permission(fmode_t mode, int perm);

/**Finally I will do kzalloc of this structure and save it in the driver field
 * of platform device->dev-> structure*/
struct pcdev_private_data {
  struct pcdev_platform_data pData;
  char *buffer;
  dev_t dev_num;
  struct cdev cdev;
};

struct pcdrv_private_data {
  int total_devices;
  dev_t device_num_base;
  struct class *class_pcd;
  struct device *device_pcd;
};

struct pcdrv_private_data pcdrv_data;

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence);
ssize_t pcd_read(struct file *filp, char __user *buff, size_t count,
                 loff_t *f_pos);
ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count,
                  loff_t *f_pos);
int pcd_open(struct inode *inode, struct file *filp);
int pcd_release(struct inode *inode, struct file *flip);

/**Trick to get the function name **/
#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt, __func__

#define MAX_DEVICES (2)

struct file_operations pcd_fops = {.open = pcd_open,
                                   .release = pcd_release,
                                   .read = pcd_read,
                                   .write = pcd_write,
                                   .llseek = pcd_lseek,
                                   .owner = THIS_MODULE

};

struct platform_device_id pcdev_ids[] = {

        [0] = {.name = "pcdev-A1x", .driver_data = 0},
        [1] = {.name = "pcdev-B1x", .driver_data = PCDEVB1x},
        [2] = {.name = "pcdev-C1x", .driver_data = PCDEVC1x},
        {}, // NULL terminated
};

struct device_config pcdev_config[] = {
        [0] = {.config_item1 = 60, .config_item2 = 21},
        [1] = {.config_item1 = 70, .config_item2 = 31},
        [2] = {.config_item1 = 80, .config_item2 = 41},
        [3] = {.config_item1 = 90, .config_item2 = 51},
};

int pcd_platform_driver_probe(struct platform_device *pDev) {
  struct pcdev_platform_data *pData;
  struct pcdev_private_data *dev_data;
  int ret = 0;

  pr_info("pcd_platform_driver_probe\n");
  pData = pDev->dev.platform_data;

  // GFP stands for Get Free Page
  // dev_data = kzalloc(sizeof(struct pcdev_private_data), GFP_KERNEL);
  dev_data =
      devm_kzalloc(&(pDev->dev), sizeof(struct pcdev_private_data), GFP_KERNEL);
  if (!dev_data) {
    pr_err("Cannot allocate memory - pcdev_private_data\n");
    return -ENOMEM;
  }
  pDev->dev.driver_data = dev_data; // If stored here in the platform Device
                                    // then we can free it in the release.

  dev_data->pData.size = pData->size;
  dev_data->pData.perm = pData->perm;
  dev_data->pData.serial_number = pData->serial_number;

  pr_info("Device serial number = %s\n", dev_data->pData.serial_number);
  pr_info("Device size = %d\n", dev_data->pData.size);
  pr_info("Device permission = %d\n", dev_data->pData.perm);

  pr_info("config_item1 = %d and config_item2 = %d\r\n",
          pcdev_config[pDev->id_entry->driver_data].config_item1,
          pcdev_config[pDev->id_entry->driver_data].config_item2);

  // dev_data->buffer = kzalloc(dev_data->pData.size, GFP_KERNEL);
  dev_data->buffer =
      devm_kzalloc(&(pDev->dev), dev_data->pData.size, GFP_KERNEL);
  if (!dev_data->buffer) {
    pr_err("Cannot allocate memory - buffer\n");
    // devm_kfree(&(pDev->dev),dev_data);  //not required - since devm_kzalloc
    // is managed resource.
    return -ENOMEM;
  }

  dev_data->dev_num = pcdrv_data.device_num_base + pDev->id;

  cdev_init(&dev_data->cdev, &pcd_fops);

  dev_data->cdev.owner = THIS_MODULE;
  ret = cdev_add(&dev_data->cdev, dev_data->dev_num, 1);
  if (ret < 0) {
    pr_err("Cdev Add failed\n");
    return ret;
  }

  pcdrv_data.device_pcd =
      device_create(pcdrv_data.class_pcd, NULL, dev_data->dev_num, NULL,
                    "pcdev-%d", pDev->id);

  if (IS_ERR(pcdrv_data.device_pcd)) {
    pr_err("Device create failed\n");
    ret = PTR_ERR(pcdrv_data.device_pcd);
    goto class_del;
  }
  pcdrv_data.total_devices++;

  pr_info("Probe was successful\n");

  return 0;

class_del:
  cdev_del(&dev_data->cdev);
  return ret;
}

int pcd_platform_driver_remove(struct platform_device *pDev) {
  struct pcdev_private_data *pPrivateData;

  pr_info("pcd_platform_driver_remove\n");
  pPrivateData = (struct pcdev_private_data *)pDev->dev.driver_data;
  device_destroy(pcdrv_data.class_pcd, pPrivateData->dev_num);
  cdev_del(&pPrivateData->cdev);
  pcdrv_data.total_devices--;
  return 0;
}

struct platform_driver pcd_platform_driver = {
    .probe = pcd_platform_driver_probe,
    .remove = pcd_platform_driver_remove,
    .id_table = pcdev_ids, /**based on ids so .name will be ignored **/
    .driver =
        {
            .name = "pseudo-char-device",
        },
};

/**Entry Point of the Kernel Module **/
/** Called when the module is inserted -insmod **/
static int __init pcd_platform_driver_init(void) {
  int ret;
  pr_info("pcd platform driver module insert\n");
  ret = alloc_chrdev_region(&pcdrv_data.device_num_base, 0, MAX_DEVICES,
                            "pcdevs");
  if (ret < 0) {
    pr_err("Alloc chrdev failed\n");
    return ret;
  }

  pcdrv_data.class_pcd = class_create(THIS_MODULE, "pcd_class");
  if (IS_ERR(pcdrv_data.class_pcd)) {
    pr_err("Error creating class\n");
    ret = PTR_ERR(pcdrv_data.class_pcd);
    unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);
    return ret;
  }

  platform_driver_register(&pcd_platform_driver);
  return 0;
}

/** Called when the module is removed - rmmod**/
static void __exit pcd_platform_driver_cleanup(void) {
  pr_info("module unloaded\n");

  platform_driver_unregister(&pcd_platform_driver);
  class_destroy(pcdrv_data.class_pcd);
  unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);
}

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence) { return 0; }

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count,
                 loff_t *f_pos) {
  return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count,
                  loff_t *f_pos) {
  return 0;
}

int pcd_open(struct inode *inode, struct file *filp) { return 0; }

int pcd_release(struct inode *inode, struct file *flip) {
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

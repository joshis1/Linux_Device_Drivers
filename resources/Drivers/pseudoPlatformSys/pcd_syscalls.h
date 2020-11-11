#ifndef __PCD_SYSCALLS__
#define __PCD_SYSCALLS__

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
/** kzalloc **/
#include <linux/slab.h>

#include "platform.h"
#include <linux/platform_device.h>
/**platform_device_id - match based out of platform_device_id **/
#include <linux/mod_devicetable.h>
#include <linux/of_device.h>


/**private static prototypes**/
//static int check_permission(fmode_t mode, int perm);

/**Finally I will do kzalloc of this structure and save it in the driver field of platform device->dev-> structure*/
struct pcdev_private_data
{
   struct pcdev_platform_data pData;
   char *buffer;
   dev_t dev_num;
   struct cdev cdev;
};

struct pcdrv_private_data 
{
   int total_devices;
   dev_t device_num_base;
   struct class *class_pcd;
   struct device *device_pcd;
};























#endif

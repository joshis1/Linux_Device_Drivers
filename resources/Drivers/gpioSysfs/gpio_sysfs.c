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

#include <linux/platform_device.h>
/**platform_device_id - match based out of platform_device_id **/
#include <linux/mod_devicetable.h>
#include <linux/of_device.h>


/**private static prototypes**/
//static int check_permission(fmode_t mode, int perm);

/**Finally I will do kzalloc of this structure and save it in the driver field of platform device->dev-> structure*/
struct gpiodev_private_data
{
   char label[20];
};

struct gpiodrv_private_data 
{
   int total_devices;
   struct class *class_gpio;
};

struct gpiodrv_private_data gpiodrv_data;

/**Trick to get the function name **/
#undef pr_fmt
#define pr_fmt(fmt) "%s : "fmt,__func__

struct of_device_id gpio_sysfs_dt_match[] = {
 { .compatible = "org,bone-gpio-sysfs" },
 {}, // NULL terminated
};


int gpio_sysfs_platform_driver_probe (struct platform_device *pDev)
{   
   return 0;
}

int gpio_sysfs_driver_remove(struct platform_device *pDev)
{ 
   return 0;
}

struct platform_driver gpiosysfs_platform_driver = {
	.probe =  gpio_sysfs_platform_driver_probe,
	.remove = gpio_sysfs_driver_remove,
	.driver = {
          .name =  "bone-gpio-sysfs",
          . of_match_table = of_match_ptr(gpio_sysfs_dt_match),
	},	
};

/**Entry Point of the Kernel Module **/
/** Called when the module is inserted -insmod **/
static int __init gpio_sysfs_init(void)
{
   int ret;
   gpiodrv_data.class_gpio = class_create(THIS_MODULE, "bone_gpios");
   ret = platform_driver_register(&gpiosysfs_platform_driver); 	
   return ret;  
}

/** Called when the module is removed - rmmod**/
static void __exit gpio_sysfs_exit(void)
{
   platform_driver_unregister(&gpiosysfs_platform_driver);
   class_destroy(gpiodrv_data.class_gpio);
   pr_info("module unloaded\n");
}

/**module registration **/
module_init(gpio_sysfs_init);
module_exit(gpio_sysfs_exit);

/*GPL - check module.h - very important */
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Shreyas Joshi");
MODULE_DESCRIPTION("GPIO Sysfs Driver - Device Tree based");
MODULE_INFO(board, "Zedboard - AVNET");


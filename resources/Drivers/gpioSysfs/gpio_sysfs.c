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

#include <linux/platform_device.h>
/**platform_device_id - match based out of platform_device_id **/
#include <linux/mod_devicetable.h>
#include <linux/of_device.h>
/** for accessing gpios **/
#include <linux/gpio/consumer.h>

/**private static prototypes**/
// static int check_permission(fmode_t mode, int perm);

/**Finally I will do kzalloc of this structure and save it in the driver field
 * of platform device->dev-> structure*/
struct gpiodev_private_data {
  char label[20];
  struct gpio_desc *desc;
  struct mutex gpio_mutex_lock;
};

struct gpiodrv_private_data {
  int total_devices;
  struct class *class_gpio;
  struct device **dev_sysfs;
};

struct gpiodrv_private_data gpiodrv_data;

/**Trick to get the function name **/
#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt, __func__

struct of_device_id gpio_sysfs_dt_match[] = {
    {.compatible = "org,bone-gpio-sysfs"}, {} // NULL terminated
};

ssize_t direction_show(struct device *dev, struct device_attribute *attr,
                       char *buf) {
  int direction;
  char *dir;
  int ret;
  struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);
  mutex_lock(&dev_data->gpio_mutex_lock);
  direction = gpiod_get_direction(dev_data->desc);
  if (direction < 0) {
    pr_err("Failed reading gpiod - direction\r\n");
    mutex_unlock(&dev_data->gpio_mutex_lock);
    return direction;
  }
  dir = direction ? "in" : "out";
  ret = sprintf(buf, "%s\n", dir);
  mutex_unlock(&dev_data->gpio_mutex_lock);
  return ret;
}

ssize_t direction_store(struct device *dev, struct device_attribute *attr,
                        const char *buf, size_t count) {
  struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);
  mutex_lock(&dev_data->gpio_mutex_lock);
  if (sysfs_streq(buf, "in")) {
    if (gpiod_direction_input(dev_data->desc)) {
      pr_err("Failed to set the direction input \r\n");
    }
  } else if (sysfs_streq(buf, "out")) {
    if (gpiod_direction_output(dev_data->desc, 0)) {
      pr_err("Failed to set the direction output\r\n");
    }
  } else {
    mutex_unlock(&dev_data->gpio_mutex_lock);
    return -EINVAL;
  }
  mutex_unlock(&dev_data->gpio_mutex_lock);
  return count;
}

ssize_t value_show(struct device *dev, struct device_attribute *attr,
                   char *buf) {
  int ret;
  int value;
  struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);
  mutex_lock(&dev_data->gpio_mutex_lock);
  value = gpiod_get_value(dev_data->desc);
  ret = sprintf(buf, "%d\n", value);
  mutex_unlock(&dev_data->gpio_mutex_lock);
  return ret;
}

ssize_t value_store(struct device *dev, struct device_attribute *attr,
                    const char *buf, size_t count) {
  struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);
  long val;
  int ret;
  mutex_lock(&dev_data->gpio_mutex_lock);
  ret = kstrtol(buf, 0, &val);
  if (ret) {
    pr_err("Incorrect value by the user\r\n");
    mutex_unlock(&dev_data->gpio_mutex_lock);
    return ret;
  }
  gpiod_set_value(dev_data->desc, val);
  mutex_unlock(&dev_data->gpio_mutex_lock);
  return count;
}

ssize_t label_show(struct device *dev, struct device_attribute *attr,
                   char *buf) {
  struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);
  int ret;
  mutex_lock(&dev_data->gpio_mutex_lock);
  ret = sprintf(buf, "%s\n", dev_data->label);
  mutex_unlock(&dev_data->gpio_mutex_lock);
  return ret;
}

static DEVICE_ATTR_RW(direction);
static DEVICE_ATTR_RW(value);
static DEVICE_ATTR_RO(label);

static struct attribute *gpio_attrs[] = {
    &dev_attr_direction.attr, &dev_attr_value.attr, &dev_attr_label.attr, NULL};

static struct attribute_group gpio_group = {.attrs = gpio_attrs};

const struct attribute_group *gpio_attr_groups[] = {&gpio_group, NULL};

int gpio_sysfs_platform_driver_probe(struct platform_device *pDev) {
  struct device_node *parent = pDev->dev.of_node;
  struct device_node *child;
  struct gpiodev_private_data *dev_data;

  struct device *dev = &pDev->dev;
  const char *name;
  int i = 0;
  int ret;

  gpiodrv_data.total_devices = of_get_child_count(parent);
  if (!gpiodrv_data.total_devices) {
    dev_err(dev, "Error - No devices found\r\n");
    return -EINVAL;
  }
  dev_info(dev, "child count = %d\r\n", gpiodrv_data.total_devices);
  gpiodrv_data.dev_sysfs = devm_kzalloc(
      &pDev->dev, sizeof(struct device *) * gpiodrv_data.total_devices,
      GFP_KERNEL);

  for_each_available_child_of_node(parent, child) {
    dev_data =
        devm_kzalloc(dev, sizeof(struct gpiodev_private_data), GFP_KERNEL);
    if (!dev_data) {
      pr_err("No memory \r\n");
      return -ENOMEM;
    }
    if (of_property_read_string(child, "label", &name)) {
      pr_err("Missing label \r\n");
      snprintf(dev_data->label, sizeof(dev_data->label), "unknown-gpio-%d", i);
    } else {
      strcpy(dev_data->label, name);
      dev_info(dev, "GPIO label = %s\n", dev_data->label);
    }
    dev_data->desc = devm_fwnode_get_gpiod_from_child(
        dev, "bone", &child->fwnode, GPIOD_ASIS, dev_data->label);
    if (IS_ERR(dev_data->desc)) {
      ret = PTR_ERR(dev_data->desc);
      if (ret == -ENONET) {
        dev_err(dev, "get gpiod failed - no entry found\r\n");
      }
      return ret;
    }
    /** gpio/gpio.txt **/
    ret = gpiod_direction_output(dev_data->desc, 0);
    if (ret) {
      dev_err(dev, "Err setting gpio direction failed = %d\r\n", i);
      return ret;
    }

    gpiodrv_data.dev_sysfs[i] =
        device_create_with_groups(gpiodrv_data.class_gpio, dev, 0, dev_data,
                                  gpio_attr_groups, dev_data->label);
    if (IS_ERR(gpiodrv_data.dev_sysfs[i])) {
      ret = PTR_ERR(gpiodrv_data.dev_sysfs[i]);
      dev_err(dev, "Error creating device with groups \r\n");
      return ret;
    }
    mutex_init(&dev_data->gpio_mutex_lock);
    i++;
  }
  return 0;
}

int gpio_sysfs_driver_remove(struct platform_device *pDev) {
  int i = 0;
  dev_info(&pDev->dev, "Removing gpio syfs driver\r\n");

  for (i = 0; i < gpiodrv_data.total_devices; i++) {
    device_unregister(gpiodrv_data.dev_sysfs[i]);
  }
  return 0;
}

struct platform_driver gpiosysfs_platform_driver = {
    .probe = gpio_sysfs_platform_driver_probe,
    .remove = gpio_sysfs_driver_remove,
    .driver =
        {
            .name = "bone-gpio-sysfs",
            .of_match_table = of_match_ptr(gpio_sysfs_dt_match),
        },
};

/**Entry Point of the Kernel Module **/
/** Called when the module is inserted -insmod **/
static int __init gpio_sysfs_init(void) {
  int ret;
  gpiodrv_data.class_gpio = class_create(THIS_MODULE, "bone_gpios");
  ret = platform_driver_register(&gpiosysfs_platform_driver);
  return ret;
}

/** Called when the module is removed - rmmod**/
static void __exit gpio_sysfs_exit(void) {
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

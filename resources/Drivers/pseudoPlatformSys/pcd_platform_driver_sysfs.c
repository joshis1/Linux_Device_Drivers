#include "pcd_syscalls.h"

struct pcdrv_private_data pcdrv_data;

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

struct of_device_id pcdev_dt_match[] = {
    {.compatible = "pcdev-A1x", .data = (void *)PCDEVA1x},
    {.compatible = "pcdev-B1x", .data = (void *)PCDEVB1x},
    {.compatible = "pcdev-C1x", .data = (void *)PCDEVC1x},
    {.compatible = "pcdev-D1x", .data = (void *)PCDEVD1x},
    {}, // NULL terminated
};

struct device_config pcdev_config[] = {
        [0] = {.config_item1 = 60, .config_item2 = 21},
        [1] = {.config_item1 = 70, .config_item2 = 31},
        [2] = {.config_item1 = 80, .config_item2 = 41},
        [3] = {.config_item1 = 90, .config_item2 = 51},
};

struct platform_device_id pcdev_ids[] = {

        [0] = {.name = "pcdev-A1x", .driver_data = 0},
        [1] = {.name = "pcdev-B1x", .driver_data = PCDEVB1x},
        [2] = {.name = "pcdev-C1x", .driver_data = PCDEVC1x},
        {}, // NULL terminated
};


static DEVICE_ATTR(max_size, S_IRUGO | S_IWUSR, show_max_size, store_max_size);
static DEVICE_ATTR(serial_num, S_IRUGO, show_serial_num, NULL);

struct attribute *pcd_attrs[] =
{
  &dev_attr_max_size.attr,
  &dev_attr_serial_num.attr,
  NULL
};

struct attribute_group pcd_attr_group =
{
    .attrs = pcd_attrs,
};

struct pcdev_platform_data *pcdev_get_platform_from_dt(struct device *dev) {
  struct device_node *dev_node = dev->of_node;
  struct pcdev_platform_data *pData;

  if (!dev_node) {
    return NULL;
  }

  pData = devm_kzalloc(dev, sizeof(struct pcdev_private_data), GFP_KERNEL);
  if (!pData) {
    dev_err(dev, "Error allocating memory\r\n");
    return ERR_PTR(-ENOMEM);
  }
  if (of_property_read_string(dev_node, "org,device-serial-num",
                              &pData->serial_number)) {
    dev_err(dev, "Error reading device-serial-num \r\n");
    return ERR_PTR(-EINVAL);
  }
  if (of_property_read_u32(dev_node, "org,size", &pData->size)) {
    dev_err(dev, "Error reading device size \r\n");
    return ERR_PTR(-EINVAL);
  }

  if (of_property_read_u32(dev_node, "org,perm", &pData->perm)) {
    dev_err(dev, "Error reading device perm \r\n");
    return ERR_PTR(-EINVAL);
  }

  return pData;
}


int pcd_sysfs_create_files(struct device *device_pcd) {
  #if 0
  int ret = 0;
  ret = sysfs_create_file(&device_pcd->kobj, &dev_attr_max_size.attr);
  if (ret < 0) {
    dev_err(device_pcd, "Error creating sys file - max size \r\n");
  }
  ret = sysfs_create_file(&device_pcd->kobj, &dev_attr_serial_num.attr);
  return ret;
  #endif
  return sysfs_create_group(&device_pcd->kobj, &pcd_attr_group);
  
}

ssize_t show_max_size(struct device *dev, struct device_attribute *attr,
                      char *buf) {
  struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);

  return sprintf(buf, "%d\n", dev_data->pData.size);
}

ssize_t store_max_size(struct device *dev, struct device_attribute *attr,
                       const char *buf, size_t count) {
  int ret;
  long result;
  struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);

  ret = kstrtol(buf,0, &result);
  if(ret) 
    return ret;
  dev_data->pData.size = result;

  dev_data->buffer = krealloc(dev_data->buffer,dev_data->pData.size, GFP_KERNEL);
  return count;

}

ssize_t show_serial_num(struct device *dev, struct device_attribute *attr,
                      char *buf) {
  struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);

  return sprintf(buf, "%s\n", dev_data->pData.serial_number);
}

int pcd_platform_driver_probe(struct platform_device *pDev) {
  struct pcdev_platform_data *pData;
  struct pcdev_private_data *dev_data;
  const struct of_device_id *of_id;
  int index = 0;
  int ret = 0;

  pr_info("pcd_platform_driver_probe\n");

  pData = pcdev_get_platform_from_dt(&(pDev->dev));

  if (!pData) {
    pr_info("trying to get the platform data from the device\r\n");
    pData = pDev->dev.platform_data;
    if (!pData) {
      pr_err("No platform data available\r\n");
      return -1;
    }
    index = pDev->id_entry->driver_data;
  } else {
    of_id = of_match_device(pDev->dev.driver->of_match_table, &pDev->dev);
    index = (long)of_id->data;
  }
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
          pcdev_config[index].config_item1, pcdev_config[index].config_item2);

  // dev_data->buffer = kzalloc(dev_data->pData.size, GFP_KERNEL);
  dev_data->buffer =
      devm_kzalloc(&(pDev->dev), dev_data->pData.size, GFP_KERNEL);
  if (!dev_data->buffer) {
    pr_err("Cannot allocate memory - buffer\n");
    // devm_kfree(&(pDev->dev),dev_data);  //not required - since devm_kzalloc
    // is managed resource.
    return -ENOMEM;
  }

  dev_data->dev_num = pcdrv_data.device_num_base + pcdrv_data.total_devices;
  cdev_init(&dev_data->cdev, &pcd_fops);

  dev_data->cdev.owner = THIS_MODULE;
  ret = cdev_add(&dev_data->cdev, dev_data->dev_num, 1);
  if (ret < 0) {
    pr_err("Cdev Add failed\n");
    return ret;
  }

  pcdrv_data.device_pcd =
      device_create(pcdrv_data.class_pcd, &pDev->dev, dev_data->dev_num, NULL,
                    "pcdev-%d", pcdrv_data.total_devices);

  if (IS_ERR(pcdrv_data.device_pcd)) {
    pr_err("Device create failed\n");
    ret = PTR_ERR(pcdrv_data.device_pcd);
    goto class_del;
  }
  pcdrv_data.total_devices++;

  ret = pcd_sysfs_create_files(pcdrv_data.device_pcd);

  if (ret) {
    pr_err("Failed to create sysfs \r\n");
    return ret;
  }

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
            .name = "pseudo-char-device", .of_match_table = pcdev_dt_match,
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

/**module registration **/
module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_cleanup);

/*GPL - check module.h - very important */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shreyas Joshi");
MODULE_DESCRIPTION("Pseudo platform device driver - Device Tree based");
MODULE_INFO(board, "Zedboard - AVNET");

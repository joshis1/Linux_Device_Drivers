#include "platform.h"
#include <linux/module.h>
#include <linux/platform_device.h>

void pcdev_release(struct device *dev);

#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt, __func__

struct pcdev_platform_data pcdev_data[] = {
        [0] = {.size = 512, .perm = RDWR, .serial_number = "PCDEVABC11"},
        [1] = {.size = 1024, .perm = RDWR, .serial_number = "PCDEVXYZ22"},
        [2] = {.size = 256, .perm = WRONLY, .serial_number = "PCDEVXYZ333"},
        [3] = {.size = 128, .perm = RONLY, .serial_number = "PCDEVXYZ444"},
};

struct platform_device platform_pcdev_1 = {
    .name = "pcdev-A1x",
    .id = 0,
    .dev =
        {
            .platform_data = &pcdev_data[0], .release = pcdev_release,
        },
};

struct platform_device platform_pcdev_2 = {
    .name = "pcdev-B1x",
    .id = 1,
    .dev =
        {
            .platform_data = &pcdev_data[1], .release = pcdev_release,
        },
};

struct platform_device platform_pcdev_3 = {
    .name = "pcdev-C1x",
    .id = 2,
    .dev =
        {
            .platform_data = &pcdev_data[2], .release = pcdev_release,
        },
};

struct platform_device platform_pcdev_4 = {
    .name = "pseudo-char-device",
    .id = 3,
    .dev =
        {
            .platform_data = &pcdev_data[3], .release = pcdev_release,
        },
};

struct platform_device *platform_pcDevs[] = {
    &platform_pcdev_1, &platform_pcdev_2, &platform_pcdev_3, &platform_pcdev_4};

void pcdev_release(struct device *dev) { pr_info("Device released\n"); }

static int __init pcdev_platform_init(void) {
  int ret = 0;

  pr_info("pcdev platform init\n");

  ret = platform_add_devices(platform_pcDevs, ARRAY_SIZE(platform_pcDevs));
  if (ret < 0) {
    pr_err("Failed to add devices - platform\n");
    goto err;
  }

#if 0

   ret = platform_device_register(&platform_pcdev_1);
   if( ret < 0) {
	   pr_err("platform pcdev_1 registration failed\n");
	   goto err;
   }
   ret = platform_device_register(&platform_pcdev_2);
   if( ret < 0) {
	   pr_err("platform pcdev_2 registration failed\n");
	   goto err;
   }
#endif
  pr_info("pcdev platform init successful\n");
  return 0;

err:
  return ret;
}

static void __exit pcdev_platform_exit(void) {
  pr_info("pcdev platform exit\n");
  platform_device_unregister(&platform_pcdev_1);
  platform_device_unregister(&platform_pcdev_2);
  platform_device_unregister(&platform_pcdev_3);
  platform_device_unregister(&platform_pcdev_4);
  pr_info("pcdev platform successfully\n");
}

module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);

MODULE_LICENSE("GPL");

#ifndef __PLATFORM__
#define __PLATFORM__

struct pcdev_platform_data
{
  int size;
  int perm;
  const char *serial_number;
};

enum pcdev_names
{
  PCDEVA1x = 0,
  PCDEVB1x,
  PCDEVC1x,
  PCDEVD1x,
};

struct device_config
{
  int config_item1;
  int config_item2;
};

#define RDWR 0x11
#define RONLY 0x10
#define WRONLY 0x01

#endif

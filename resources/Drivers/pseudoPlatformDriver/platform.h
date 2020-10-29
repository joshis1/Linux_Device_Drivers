struct pcdev_platform_data {
 int size;
 int perm;
 const char *serial_number;
};


#define RDWR   0x11
#define RONLY  0x10
#define WRONLY 0x01
